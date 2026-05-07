/**
 * Application FSM extracted from runtime orchestration.
 *
 * Features (EN):
 * - Applies RFID and HX711 events to AppState.
 * - Encapsulates transition logic and timeout handling.
 * - Avoids FreeRTOS, queues, display, and sensor driver dependencies.
 *
 * Funkcje (PL):
 * - Aplikuje zdarzenia RFID i HX711 do AppState.
 * - Hermetyzuje logike przejsc oraz timeouty.
 * - Nie zalezy od FreeRTOS, kolejek, wyswietlacza ani driverow.
 *
 * File: components/app/src/app_fsm.cpp
 */

#include "app/app_fsm.hpp"

#include <cstring>

namespace app {
namespace {
constexpr uint32_t kAppStartupTimeoutMs = 5000;

void clear_uid(uint8_t *uid) {
  std::memset(uid, 0, kMaxRfidUidLength);
}
}  // namespace

void AppFsm::reset(AppState &state, uint32_t now_ms) const {
  state = AppState{};
  state.current_mode = AppMode::kBoot;
  state.rfid_status = RfidStatus::kBooting;
  state.hx711_status = Hx711Status::kBooting;
  state.rfid_last_change_ms = now_ms;
  state.hx711_last_sample_ms = now_ms;
}

bool AppFsm::transition_to(AppState &state, AppMode next_mode) {
  if (state.current_mode == next_mode) {
    return false;
  }

  state.current_mode = next_mode;
  return true;
}

void AppFsm::sync_mode(AppState &state) {
  if (state.current_mode == AppMode::kError || state.current_mode == AppMode::kCalibration) {
    return;
  }

  if (state.rfid_status == RfidStatus::kReaderMissing || state.hx711_status == Hx711Status::kNotFound) {
    (void)transition_to(state, AppMode::kError);
    return;
  }

  if (state.rfid_status == RfidStatus::kBooting || state.hx711_status == Hx711Status::kBooting) {
    (void)transition_to(state, AppMode::kBoot);
    return;
  }

  if (state.rfid_status == RfidStatus::kCardPresent) {
    if (state.rfid_has_uid && state.hx711_has_sample) {
      (void)transition_to(state, AppMode::kMeasuring);
      return;
    }

    (void)transition_to(state, AppMode::kTagDetected);
    return;
  }

  (void)transition_to(state, AppMode::kIdle);
}

bool AppFsm::handle_event(AppState &state, const RfidEvent &event) const {
  state.rfid_last_change_ms = event.timestamp_ms;

  switch (event.kind) {
    case RfidEvent::Kind::kReaderMissing:
      state.rfid_status = RfidStatus::kReaderMissing;
      state.rfid_has_uid = false;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_firmware_version = 0;
      state.rfid_uid_length = 0;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      clear_uid(state.rfid_uid);
      (void)transition_to(state, AppMode::kError);
      return true;

    case RfidEvent::Kind::kReaderReady:
      state.rfid_status = RfidStatus::kWaitingForCard;
      state.rfid_has_uid = false;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_firmware_version = event.firmware_version;
      state.rfid_uid_length = 0;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      clear_uid(state.rfid_uid);
      sync_mode(state);
      return true;

    case RfidEvent::Kind::kWaitingForCard:
      state.rfid_status = RfidStatus::kWaitingForCard;
      state.rfid_has_uid = false;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_uid_length = 0;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      if (event.firmware_version != 0) {
        state.rfid_firmware_version = event.firmware_version;
      }
      clear_uid(state.rfid_uid);
      sync_mode(state);
      return true;

    case RfidEvent::Kind::kCardRemoved:
      state.rfid_status = RfidStatus::kCardRemoved;
      state.rfid_has_uid = false;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_uid_length = 0;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      clear_uid(state.rfid_uid);
      sync_mode(state);
      return true;

    case RfidEvent::Kind::kCardPresent:
      state.rfid_status = RfidStatus::kCardPresent;
      state.rfid_has_uid = event.has_uid;
      state.rfid_usage_available = event.usage_available;
      state.rfid_page_read_failed = event.page_read_failed;
      state.rfid_firmware_version = event.firmware_version;
      state.rfid_uid_length = event.uid_length;
      state.rfid_usage_seconds = event.usage_seconds;
      state.rfid_life_percent = event.life_percent;
      clear_uid(state.rfid_uid);
      if (event.has_uid) {
        const uint8_t copy_length = event.uid_length > kMaxRfidUidLength ? kMaxRfidUidLength : event.uid_length;
        std::memcpy(state.rfid_uid, event.uid, copy_length);
      }
      sync_mode(state);
      return true;
  }

  return false;
}

bool AppFsm::handle_event(AppState &state, const WeightEvent &event) const {
  state.hx711_last_sample_ms = event.timestamp_ms;

  switch (event.kind) {
    case WeightEvent::Kind::kNotFound:
      state.hx711_status = Hx711Status::kNotFound;
      state.hx711_has_sample = false;
      (void)transition_to(state, AppMode::kError);
      return true;

    case WeightEvent::Kind::kReady:
      state.hx711_status = Hx711Status::kReady;
      state.hx711_has_sample = false;
      sync_mode(state);
      return true;

    case WeightEvent::Kind::kSample:
      state.hx711_status = Hx711Status::kReady;
      state.hx711_has_sample = event.has_sample;
      state.hx711_raw_value = event.raw_value;
      sync_mode(state);
      return true;
  }

  return false;
}

bool AppFsm::handle_timeouts(AppState &state, uint32_t now_ms) const {
  if (state.current_mode == AppMode::kError || state.current_mode == AppMode::kCalibration) {
    return false;
  }

  if (state.rfid_status == RfidStatus::kBooting && now_ms - state.rfid_last_change_ms > kAppStartupTimeoutMs) {
    return transition_to(state, AppMode::kError);
  }

  if (state.hx711_status == Hx711Status::kBooting && now_ms - state.hx711_last_sample_ms > kAppStartupTimeoutMs) {
    return transition_to(state, AppMode::kError);
  }

  return false;
}
}  // namespace app
