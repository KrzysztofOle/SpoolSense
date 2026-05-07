/**
 * Application orchestration layer.
 *
 * Features (EN):
 * - Owns the firmware lifecycle and FreeRTOS task startup.
 * - Separates RFID, HX711, UI, and diagnostics responsibilities.
 *
 * Funkcje (PL):
 * - Zarzadza cyklem zycia firmware i startem taskow FreeRTOS.
 * - Rozdziela odpowiedzialnosci RFID, HX711, UI i diagnostyki.
 *
 * File: components/app/src/app.cpp
 */

#include "app/app.hpp"

#include <Arduino.h>
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

#include "board/board.hpp"
#include "diagnostics/diagnostics.hpp"
#include "display/display.hpp"
#include "hx711/hx711_monitor.hpp"
#include "pn532/pn532_reader.hpp"

namespace app {
namespace {
constexpr uint32_t kRfidPollIntervalMs = 400;
constexpr uint32_t kRfidCardGoneTimeoutMs = 1600;
constexpr uint32_t kRfidInitRetryMs = 1500;
constexpr uint16_t kRfidReadTimeoutMs = 50;
constexpr uint32_t kHx711SampleIntervalMs = 700;
constexpr uint32_t kHx711InitRetryMs = 1500;
constexpr uint32_t kUiRefreshIntervalMs = 250;
constexpr uint32_t kDiagnosticsIntervalMs = 5000;
constexpr uint8_t kMaxUidLength = 10;

bool same_uid(const uint8_t *lhs, const uint8_t *rhs, uint8_t uid_length) {
  if (uid_length == 0) {
    return false;
  }

  return std::memcmp(lhs, rhs, uid_length) == 0;
}

bool same_state(const App::RuntimeState &lhs, const App::RuntimeState &rhs) {
  return lhs.rfid_status == rhs.rfid_status && lhs.rfid_has_uid == rhs.rfid_has_uid &&
         lhs.rfid_usage_available == rhs.rfid_usage_available &&
         lhs.rfid_page_read_failed == rhs.rfid_page_read_failed &&
         lhs.rfid_uid_length == rhs.rfid_uid_length &&
         lhs.rfid_firmware_version == rhs.rfid_firmware_version &&
         lhs.rfid_usage_seconds == rhs.rfid_usage_seconds &&
         lhs.rfid_life_percent == rhs.rfid_life_percent &&
         lhs.rfid_last_change_ms == rhs.rfid_last_change_ms &&
         std::memcmp(lhs.rfid_uid, rhs.rfid_uid, sizeof(lhs.rfid_uid)) == 0;
}

void copy_rfid_fields(const App::RuntimeState &source, App::RuntimeState *target) {
  target->rfid_status = source.rfid_status;
  target->rfid_has_uid = source.rfid_has_uid;
  target->rfid_usage_available = source.rfid_usage_available;
  target->rfid_page_read_failed = source.rfid_page_read_failed;
  target->rfid_uid_length = source.rfid_uid_length;
  target->rfid_firmware_version = source.rfid_firmware_version;
  target->rfid_usage_seconds = source.rfid_usage_seconds;
  target->rfid_life_percent = source.rfid_life_percent;
  target->rfid_last_change_ms = source.rfid_last_change_ms;
  if (source.rfid_has_uid) {
    std::memcpy(target->rfid_uid, source.rfid_uid, sizeof(target->rfid_uid));
  } else {
    std::memset(target->rfid_uid, 0, sizeof(target->rfid_uid));
  }
}

void copy_hx711_fields(const App::RuntimeState &source, App::RuntimeState *target) {
  target->hx711_status = source.hx711_status;
  target->hx711_has_sample = source.hx711_has_sample;
  target->hx711_raw_value = source.hx711_raw_value;
  target->hx711_last_sample_ms = source.hx711_last_sample_ms;
}

void clear_uid(uint8_t *uid) {
  std::memset(uid, 0, kMaxUidLength);
}

const char *rfid_status_text(RfidStatus status) {
  switch (status) {
    case RfidStatus::kBooting:
      return "RFID: booting";
    case RfidStatus::kReaderMissing:
      return "RFID: missing";
    case RfidStatus::kWaitingForCard:
      return "RFID: waiting";
    case RfidStatus::kCardPresent:
      return "RFID: card present";
    case RfidStatus::kCardRemoved:
      return "RFID: card removed";
  }

  return "RFID: unknown";
}

const char *hx711_status_text(Hx711Status status) {
  switch (status) {
    case Hx711Status::kBooting:
      return "HX711: booting";
    case Hx711Status::kNotFound:
      return "HX711: missing";
    case Hx711Status::kReady:
      return "HX711: ready";
  }

  return "HX711: unknown";
}

void render_state(const App::RuntimeState &state) {
  if (state.rfid_status == RfidStatus::kBooting) {
    display::show_text("PN532 init test", "Starting runtime");
    return;
  }

  if (state.rfid_status == RfidStatus::kReaderMissing) {
    display::show_text("PN532 not found");
    return;
  }

  if (state.rfid_status == RfidStatus::kCardRemoved) {
    display::show_card_removed();
    return;
  }

  if (state.rfid_has_uid) {
    if (state.rfid_usage_available) {
      display::show_uid_and_usage(state.rfid_uid, state.rfid_uid_length, state.rfid_usage_seconds,
                                  state.rfid_life_percent);
      return;
    }

    display::show_uid_and_type(state.rfid_uid, state.rfid_uid_length);
    if (state.rfid_page_read_failed) {
      display::show_page_read_failed();
    }
    return;
  }

  display::show_text("PN532 init OK", "Waiting for card");
}
}  // namespace

void App::begin() {
  Serial.begin(115200);
  M5.begin();
  board::begin_i2c_mutex();
  display::begin();

  reset_state();
  start_tasks();
}

void App::restart() {
  stop_tasks();
  reset_state();
  start_tasks();
}

void App::start_tasks() {
  stop_tasks();

  BaseType_t result = xTaskCreate(rfid_task_entry, "rfid_task", 6144, this, 3, &rfid_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("RFID task start failed");
    stop_tasks();
    return;
  }

  result = xTaskCreate(hx711_task_entry, "hx711_task", 4096, this, 2, &hx711_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("HX711 task start failed");
    stop_tasks();
    return;
  }

  result = xTaskCreate(ui_task_entry, "ui_task", 4096, this, 1, &ui_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("UI task start failed");
    stop_tasks();
    return;
  }

  result = xTaskCreate(diagnostics_task_entry, "diagnostics_task", 4096, this, 1,
                       &diagnostics_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("Diagnostics task start failed");
    stop_tasks();
    return;
  }
}

void App::stop_tasks() {
  if (rfid_task_handle_ != nullptr) {
    vTaskDelete(rfid_task_handle_);
    rfid_task_handle_ = nullptr;
  }

  if (hx711_task_handle_ != nullptr) {
    vTaskDelete(hx711_task_handle_);
    hx711_task_handle_ = nullptr;
  }

  if (ui_task_handle_ != nullptr) {
    vTaskDelete(ui_task_handle_);
    ui_task_handle_ = nullptr;
  }

  if (diagnostics_task_handle_ != nullptr) {
    vTaskDelete(diagnostics_task_handle_);
    diagnostics_task_handle_ = nullptr;
  }
}

void App::reset_state() {
  if (state_mutex_ == nullptr) {
    state_mutex_ = xSemaphoreCreateMutex();
  }

  RuntimeState state{};
  state.rfid_status = RfidStatus::kBooting;
  state.hx711_status = Hx711Status::kBooting;
  state.rfid_last_change_ms = millis();
  publish_state(state);
}

App::RuntimeState App::snapshot_state() const {
  RuntimeState copy = runtime_state_;
  if (state_mutex_ == nullptr) {
    return copy;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    copy = runtime_state_;
    xSemaphoreGive(state_mutex_);
  }

  return copy;
}

void App::publish_state(const RuntimeState &state) {
  if (state_mutex_ == nullptr) {
    runtime_state_ = state;
    return;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    runtime_state_ = state;
    xSemaphoreGive(state_mutex_);
  }
}

void App::publish_rfid_state(const RuntimeState &state) {
  if (state_mutex_ == nullptr) {
    copy_rfid_fields(state, &runtime_state_);
    return;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    copy_rfid_fields(state, &runtime_state_);
    xSemaphoreGive(state_mutex_);
  }
}

void App::publish_hx711_state(const RuntimeState &state) {
  if (state_mutex_ == nullptr) {
    copy_hx711_fields(state, &runtime_state_);
    return;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    copy_hx711_fields(state, &runtime_state_);
    xSemaphoreGive(state_mutex_);
  }
}

void App::rfid_task_entry(void *arg) {
  static_cast<App *>(arg)->rfid_task_loop();
}

void App::hx711_task_entry(void *arg) {
  static_cast<App *>(arg)->hx711_task_loop();
}

void App::ui_task_entry(void *arg) {
  static_cast<App *>(arg)->ui_task_loop();
}

void App::diagnostics_task_entry(void *arg) {
  static_cast<App *>(arg)->diagnostics_task_loop();
}

void App::rfid_task_loop() {
  Pn532Reader reader;
  RuntimeState state = snapshot_state();
  uint8_t last_uid[kMaxUidLength] = {};
  uint8_t last_uid_length = 0;
  bool has_last_uid = false;
  uint32_t next_init_retry_at_ms = 0;
  uint32_t next_poll_at_ms = 0;
  uint32_t last_card_seen_at_ms = 0;

  for (;;) {
    const uint32_t now = millis();

    if (state.rfid_status == RfidStatus::kBooting || state.rfid_status == RfidStatus::kReaderMissing) {
      if (now < next_init_retry_at_ms) {
        vTaskDelay(pdMS_TO_TICKS(25));
        continue;
      }

      diagnostics::log_line("PN532 init test");
      if (!reader.begin()) {
        diagnostics::log_line("PN532 not found");
        state.rfid_status = RfidStatus::kReaderMissing;
        state.rfid_has_uid = false;
        state.rfid_usage_available = false;
        state.rfid_page_read_failed = false;
        state.rfid_firmware_version = 0;
        state.rfid_uid_length = 0;
        state.rfid_usage_seconds = 0;
        state.rfid_life_percent = 0;
        state.rfid_last_change_ms = now;
        publish_rfid_state(state);
        next_init_retry_at_ms = now + kRfidInitRetryMs;
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }

      state.rfid_status = RfidStatus::kWaitingForCard;
      state.rfid_firmware_version = reader.firmware_version();
      state.rfid_last_change_ms = now;
      publish_rfid_state(state);

      char version_line[40] = {};
      std::snprintf(version_line, sizeof(version_line), "PN532 FW: 0x%08lX",
                    static_cast<unsigned long>(reader.firmware_version()));
      diagnostics::log_line(version_line);
      diagnostics::log_line("PN532 init OK");

      next_init_retry_at_ms = 0;
      next_poll_at_ms = now;
      vTaskDelay(pdMS_TO_TICKS(25));
      continue;
    }

    if (has_last_uid && (now - last_card_seen_at_ms >= kRfidCardGoneTimeoutMs)) {
      has_last_uid = false;
      last_uid_length = 0;
      clear_uid(last_uid);
      state.rfid_status = RfidStatus::kCardRemoved;
      state.rfid_has_uid = false;
      state.rfid_uid_length = 0;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      state.rfid_last_change_ms = now;
      publish_rfid_state(state);
      diagnostics::log_line("Card removed");
    }

    if (state.rfid_status == RfidStatus::kCardRemoved &&
        (now - state.rfid_last_change_ms >= 1000U)) {
      state.rfid_status = RfidStatus::kWaitingForCard;
      state.rfid_last_change_ms = now;
      publish_rfid_state(state);
    }

    if (now < next_poll_at_ms) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    next_poll_at_ms = now + kRfidPollIntervalMs;

    uint8_t uid[kMaxUidLength] = {};
    uint8_t uid_length = 0;
    if (!reader.read_passive_target(uid, &uid_length, kRfidReadTimeoutMs)) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    last_card_seen_at_ms = now;

    if (has_last_uid && uid_length == last_uid_length && same_uid(uid, last_uid, uid_length)) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    std::memcpy(last_uid, uid, uid_length);
    last_uid_length = uid_length;
    has_last_uid = true;

    state.rfid_status = RfidStatus::kCardPresent;
    state.rfid_has_uid = true;
    state.rfid_uid_length = uid_length;
    state.rfid_page_read_failed = false;
    state.rfid_usage_available = false;
    state.rfid_last_change_ms = now;
    std::memset(state.rfid_uid, 0, sizeof(state.rfid_uid));
    std::memcpy(state.rfid_uid, uid, uid_length);

    uint8_t usage_page[4] = {};
    if (reader.read_sonicare_usage_page(usage_page)) {
      const uint16_t raw_count = diagnostics::read_little_endian_u16(usage_page);
      state.rfid_usage_seconds = diagnostics::sonicare_counts_to_seconds(raw_count);
      state.rfid_life_percent = diagnostics::estimate_life_percent(state.rfid_usage_seconds);
      state.rfid_usage_available = true;
      diagnostics::log_uid_and_usage(uid, uid_length, usage_page, state.rfid_usage_seconds,
                                     state.rfid_life_percent);
    } else {
      diagnostics::log_uid_and_type(uid, uid_length);
      diagnostics::log_line("Page 24: read failed");
      state.rfid_page_read_failed = true;
    }

    publish_rfid_state(state);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void App::hx711_task_loop() {
  Hx711Monitor monitor;
  RuntimeState state = snapshot_state();
  uint32_t next_init_retry_at_ms = 0;
  uint32_t next_sample_at_ms = 0;
  bool init_reported = false;

  monitor.begin();

  for (;;) {
    const uint32_t now = millis();

    if (!monitor.is_ready()) {
      if (!init_reported || state.hx711_status != Hx711Status::kNotFound) {
        diagnostics::log_line("HX711 not found");
      }
      init_reported = true;
      state.hx711_status = Hx711Status::kNotFound;
      state.hx711_has_sample = false;
      state.hx711_last_sample_ms = now;
      publish_hx711_state(state);
      next_init_retry_at_ms = now + kHx711InitRetryMs;
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (state.hx711_status != Hx711Status::kReady) {
      diagnostics::log_line("HX711 init OK");
      init_reported = true;
      state.hx711_status = Hx711Status::kReady;
      state.hx711_last_sample_ms = now;
      publish_hx711_state(state);
      next_sample_at_ms = now;
    }

    if (now < next_sample_at_ms) {
      if (now < next_init_retry_at_ms) {
        vTaskDelay(pdMS_TO_TICKS(10));
      } else {
        vTaskDelay(pdMS_TO_TICKS(25));
      }
      continue;
    }

    long raw_value = 0;
    if (monitor.read_raw(&raw_value)) {
      state.hx711_has_sample = true;
      state.hx711_raw_value = raw_value;
      state.hx711_last_sample_ms = now;
      publish_hx711_state(state);
      diagnostics::log_raw_weight(raw_value);
      next_sample_at_ms = now + kHx711SampleIntervalMs;
    } else {
      state.hx711_status = Hx711Status::kNotFound;
      state.hx711_has_sample = false;
      publish_hx711_state(state);
      diagnostics::log_line("HX711 not found");
      next_init_retry_at_ms = now + kHx711InitRetryMs;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void App::ui_task_loop() {
  RuntimeState last_rendered_state{};

  for (;;) {
    const RuntimeState state = snapshot_state();
    if (!same_state(state, last_rendered_state)) {
      render_state(state);
      last_rendered_state = state;
    }

    vTaskDelay(pdMS_TO_TICKS(kUiRefreshIntervalMs));
  }
}

void App::diagnostics_task_loop() {
  for (;;) {
    const RuntimeState state = snapshot_state();

    diagnostics::log_line("Runtime status");
    diagnostics::log_line(rfid_status_text(state.rfid_status));
    diagnostics::log_line(hx711_status_text(state.hx711_status));

    if (state.rfid_has_uid) {
      diagnostics::log_uid_and_type(state.rfid_uid, state.rfid_uid_length);
    }

    if (state.rfid_usage_available) {
      diagnostics::log_line("RFID usage available");
    }

    if (state.hx711_has_sample) {
      diagnostics::log_raw_weight(state.hx711_raw_value);
    }

    vTaskDelay(pdMS_TO_TICKS(kDiagnosticsIntervalMs));
  }
}
}  // namespace app
