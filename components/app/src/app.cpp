/**
 * Application orchestration layer.
 *
 * Features (EN):
 * - Owns the firmware lifecycle and FreeRTOS task startup.
 * - Routes RFID and HX711 events through queues and publishes UI snapshots.
 *
 * Funkcje (PL):
 * - Zarzadza cyklem zycia firmware i startem taskow FreeRTOS.
 * - Przekierowuje zdarzenia RFID i HX711 przez kolejki oraz publikuje snapshoty UI.
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
constexpr uint32_t kDiagnosticsIntervalMs = 5000;

bool same_uid(const uint8_t *lhs, const uint8_t *rhs, uint8_t uid_length) {
  if (uid_length == 0) {
    return false;
  }

  return std::memcmp(lhs, rhs, uid_length) == 0;
}

UiState to_ui_state(const AppState &state) {
  UiState ui_state{};
  ui_state.rfid_status = state.rfid_status;
  ui_state.hx711_status = state.hx711_status;
  ui_state.rfid_has_uid = state.rfid_has_uid;
  ui_state.rfid_usage_available = state.rfid_usage_available;
  ui_state.rfid_page_read_failed = state.rfid_page_read_failed;
  ui_state.hx711_has_sample = state.hx711_has_sample;
  ui_state.rfid_uid_length = state.rfid_uid_length;
  ui_state.rfid_firmware_version = state.rfid_firmware_version;
  ui_state.rfid_usage_seconds = state.rfid_usage_seconds;
  ui_state.rfid_life_percent = state.rfid_life_percent;
  ui_state.hx711_raw_value = state.hx711_raw_value;
  ui_state.rfid_last_change_ms = state.rfid_last_change_ms;
  ui_state.hx711_last_sample_ms = state.hx711_last_sample_ms;
  if (state.rfid_has_uid) {
    std::memcpy(ui_state.rfid_uid, state.rfid_uid, sizeof(ui_state.rfid_uid));
  }
  return ui_state;
}

bool same_state(const UiState &lhs, const UiState &rhs) {
  return lhs.rfid_status == rhs.rfid_status && lhs.rfid_has_uid == rhs.rfid_has_uid &&
         lhs.rfid_usage_available == rhs.rfid_usage_available &&
         lhs.rfid_page_read_failed == rhs.rfid_page_read_failed &&
         lhs.rfid_uid_length == rhs.rfid_uid_length &&
         lhs.rfid_usage_seconds == rhs.rfid_usage_seconds &&
         lhs.rfid_life_percent == rhs.rfid_life_percent &&
         std::memcmp(lhs.rfid_uid, rhs.rfid_uid, sizeof(lhs.rfid_uid)) == 0;
}

void clear_uid(uint8_t *uid) {
  std::memset(uid, 0, kMaxRfidUidLength);
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

void render_state(const UiState &state) {
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

  start_tasks();
}

void App::restart() {
  stop_tasks();
  reset_state();
  start_tasks();
}

void App::start_tasks() {
  stop_tasks();

  if (rfid_event_queue_ == nullptr) {
    rfid_event_queue_ = xQueueCreate(kRfidEventQueueLength, sizeof(RfidEvent));
  } else {
    xQueueReset(rfid_event_queue_);
  }

  if (weight_event_queue_ == nullptr) {
    weight_event_queue_ = xQueueCreate(kWeightEventQueueLength, sizeof(WeightEvent));
  } else {
    xQueueReset(weight_event_queue_);
  }

  if (ui_state_queue_ == nullptr) {
    ui_state_queue_ = xQueueCreate(kUiStateQueueLength, sizeof(UiState));
  } else {
    xQueueReset(ui_state_queue_);
  }

  if (rfid_event_queue_ == nullptr || weight_event_queue_ == nullptr || ui_state_queue_ == nullptr) {
    diagnostics::log_line("Queue allocation failed");
    stop_tasks();
    return;
  }

  reset_state();

  BaseType_t result = xTaskCreate(app_task_entry, "app_task", 6144, this, 4, &app_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("App task start failed");
    stop_tasks();
    return;
  }

  result = xTaskCreate(rfid_task_entry, "rfid_task", 6144, this, 3, &rfid_task_handle_);
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
  if (app_task_handle_ != nullptr) {
    vTaskDelete(app_task_handle_);
    app_task_handle_ = nullptr;
  }

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

  AppState state{};
  state.rfid_status = RfidStatus::kBooting;
  state.hx711_status = Hx711Status::kBooting;
  state.rfid_last_change_ms = millis();
  state.hx711_last_sample_ms = millis();
  publish_state(state);

  if (ui_state_queue_ != nullptr) {
    publish_ui_state(snapshot_ui_state());
  }
}

AppState App::snapshot_state() const {
  AppState copy = app_state_;
  if (state_mutex_ == nullptr) {
    return copy;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    copy = app_state_;
    xSemaphoreGive(state_mutex_);
  }

  return copy;
}

UiState App::snapshot_ui_state() const {
  return to_ui_state(snapshot_state());
}

void App::publish_state(const AppState &state) {
  if (state_mutex_ == nullptr) {
    app_state_ = state;
    return;
  }

  if (xSemaphoreTake(state_mutex_, portMAX_DELAY) == pdTRUE) {
    app_state_ = state;
    xSemaphoreGive(state_mutex_);
  }
}

void App::publish_ui_state(const UiState &state) {
  if (ui_state_queue_ == nullptr) {
    return;
  }

  (void)xQueueOverwrite(ui_state_queue_, &state);
}

bool App::publish_rfid_event(const RfidEvent &event) {
  if (rfid_event_queue_ == nullptr) {
    return false;
  }

  return xQueueSendToBack(rfid_event_queue_, &event, 0) == pdTRUE;
}

bool App::publish_weight_event(const WeightEvent &event) {
  if (weight_event_queue_ == nullptr) {
    return false;
  }

  return xQueueSendToBack(weight_event_queue_, &event, 0) == pdTRUE;
}

bool App::receive_rfid_event(RfidEvent *event) {
  if (event == nullptr || rfid_event_queue_ == nullptr) {
    return false;
  }

  return xQueueReceive(rfid_event_queue_, event, 0) == pdTRUE;
}

bool App::receive_weight_event(WeightEvent *event) {
  if (event == nullptr || weight_event_queue_ == nullptr) {
    return false;
  }

  return xQueueReceive(weight_event_queue_, event, 0) == pdTRUE;
}

bool App::ui_state_changed(const UiState &lhs, const UiState &rhs) const {
  return !same_state(lhs, rhs);
}

void App::handle_rfid_event(const RfidEvent &event) {
  AppState state = snapshot_state();
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
      diagnostics::log_line("PN532 not found");
      break;

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
      {
        char version_line[40] = {};
        std::snprintf(version_line, sizeof(version_line), "PN532 FW: 0x%08lX",
                      static_cast<unsigned long>(event.firmware_version));
        diagnostics::log_line(version_line);
        diagnostics::log_line("PN532 init OK");
      }
      break;

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
      break;

    case RfidEvent::Kind::kCardRemoved:
      state.rfid_status = RfidStatus::kCardRemoved;
      state.rfid_has_uid = false;
      state.rfid_usage_available = false;
      state.rfid_page_read_failed = false;
      state.rfid_uid_length = 0;
      state.rfid_usage_seconds = 0;
      state.rfid_life_percent = 0;
      clear_uid(state.rfid_uid);
      diagnostics::log_line("Card removed");
      break;

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

      if (event.usage_available) {
        diagnostics::log_uid_and_usage(event.uid, event.uid_length, event.usage_page,
                                       event.usage_seconds, event.life_percent);
      } else {
        diagnostics::log_uid_and_type(event.uid, event.uid_length);
        if (event.page_read_failed) {
          diagnostics::log_line("Page 24: read failed");
        }
      }
      break;
  }

  publish_state(state);
}

void App::handle_weight_event(const WeightEvent &event) {
  AppState state = snapshot_state();
  state.hx711_last_sample_ms = event.timestamp_ms;

  switch (event.kind) {
    case WeightEvent::Kind::kNotFound:
      state.hx711_status = Hx711Status::kNotFound;
      state.hx711_has_sample = false;
      diagnostics::log_line("HX711 not found");
      break;

    case WeightEvent::Kind::kReady:
      state.hx711_status = Hx711Status::kReady;
      state.hx711_has_sample = false;
      diagnostics::log_line("HX711 init OK");
      break;

    case WeightEvent::Kind::kSample:
      state.hx711_status = Hx711Status::kReady;
      state.hx711_has_sample = event.has_sample;
      state.hx711_raw_value = event.raw_value;
      if (event.has_sample) {
        diagnostics::log_raw_weight(event.raw_value);
      }
      break;
  }

  publish_state(state);
}

void App::app_task_entry(void *arg) {
  static_cast<App *>(arg)->app_task_loop();
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

void App::app_task_loop() {
  UiState last_published_state = snapshot_ui_state();

  for (;;) {
    bool handled_event = false;

    RfidEvent rfid_event{};
    while (receive_rfid_event(&rfid_event)) {
      handle_rfid_event(rfid_event);
      handled_event = true;
    }

    WeightEvent weight_event{};
    while (receive_weight_event(&weight_event)) {
      handle_weight_event(weight_event);
      handled_event = true;
    }

    if (handled_event) {
      const UiState current_state = snapshot_ui_state();
      if (ui_state_changed(current_state, last_published_state)) {
        publish_ui_state(current_state);
        last_published_state = current_state;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void App::rfid_task_loop() {
  Pn532Reader reader;
  uint8_t last_uid[kMaxRfidUidLength] = {};
  uint8_t last_uid_length = 0;
  bool has_last_uid = false;
  uint32_t next_init_retry_at_ms = 0;
  uint32_t next_poll_at_ms = 0;
  uint32_t last_card_seen_at_ms = 0;
  uint32_t card_removed_reported_at_ms = 0;
  bool reader_ready = false;
  bool card_removed_reported = false;

  for (;;) {
    const uint32_t now = millis();

    if (!reader_ready) {
      if (now < next_init_retry_at_ms) {
        vTaskDelay(pdMS_TO_TICKS(25));
        continue;
      }

      if (!reader.begin()) {
        RfidEvent event{};
        event.kind = RfidEvent::Kind::kReaderMissing;
        event.timestamp_ms = now;
        if (!publish_rfid_event(event)) {
          vTaskDelay(pdMS_TO_TICKS(50));
        }
        next_init_retry_at_ms = now + kRfidInitRetryMs;
        reader_ready = false;
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }

      RfidEvent event{};
      event.kind = RfidEvent::Kind::kReaderReady;
      event.firmware_version = reader.firmware_version();
      event.timestamp_ms = now;
      if (!publish_rfid_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }

      reader_ready = true;
      next_init_retry_at_ms = 0;
      next_poll_at_ms = now;
      vTaskDelay(pdMS_TO_TICKS(25));
      continue;
    }

    if (has_last_uid && (now - last_card_seen_at_ms >= kRfidCardGoneTimeoutMs)) {
      has_last_uid = false;
      last_uid_length = 0;
      clear_uid(last_uid);

      RfidEvent event{};
      event.kind = RfidEvent::Kind::kCardRemoved;
      event.timestamp_ms = now;
      if (!publish_rfid_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      card_removed_reported = true;
      card_removed_reported_at_ms = now;
    }

    if (card_removed_reported && (now - card_removed_reported_at_ms >= 1000U)) {
      RfidEvent event{};
      event.kind = RfidEvent::Kind::kWaitingForCard;
      event.timestamp_ms = now;
      if (!publish_rfid_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      card_removed_reported = false;
    }

    if (now < next_poll_at_ms) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    next_poll_at_ms = now + kRfidPollIntervalMs;

    uint8_t uid[kMaxRfidUidLength] = {};
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
    card_removed_reported = false;

    RfidEvent event{};
    event.kind = RfidEvent::Kind::kCardPresent;
    event.has_uid = true;
    event.uid_length = uid_length;
    event.timestamp_ms = now;
    std::memset(event.uid, 0, sizeof(event.uid));
    std::memcpy(event.uid, uid, uid_length);

    uint8_t usage_page[4] = {};
    if (reader.read_sonicare_usage_page(usage_page)) {
      const uint16_t raw_count = diagnostics::read_little_endian_u16(usage_page);
      event.usage_available = true;
      event.page_read_failed = false;
      event.usage_seconds = diagnostics::sonicare_counts_to_seconds(raw_count);
      event.life_percent = diagnostics::estimate_life_percent(event.usage_seconds);
      std::memcpy(event.usage_page, usage_page, sizeof(event.usage_page));
    } else {
      event.usage_available = false;
      event.page_read_failed = true;
      std::memset(event.usage_page, 0, sizeof(event.usage_page));
    }

    if (!publish_rfid_event(event)) {
      vTaskDelay(pdMS_TO_TICKS(50));
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void App::hx711_task_loop() {
  Hx711Monitor monitor;
  uint32_t next_init_retry_at_ms = 0;
  uint32_t next_sample_at_ms = 0;
  bool ready_reported = false;

  monitor.begin();

  for (;;) {
    const uint32_t now = millis();

    if (!monitor.is_ready()) {
      if (now < next_init_retry_at_ms) {
        vTaskDelay(pdMS_TO_TICKS(25));
        continue;
      }

      WeightEvent event{};
      event.kind = WeightEvent::Kind::kNotFound;
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }

      ready_reported = false;
      next_init_retry_at_ms = now + kHx711InitRetryMs;
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (!ready_reported) {
      WeightEvent event{};
      event.kind = WeightEvent::Kind::kReady;
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      ready_reported = true;
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
      WeightEvent event{};
      event.kind = WeightEvent::Kind::kSample;
      event.has_sample = true;
      event.raw_value = raw_value;
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      next_sample_at_ms = now + kHx711SampleIntervalMs;
    } else {
      WeightEvent event{};
      event.kind = WeightEvent::Kind::kNotFound;
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      ready_reported = false;
      next_init_retry_at_ms = now + kHx711InitRetryMs;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void App::ui_task_loop() {
  UiState last_rendered_state{};
  bool has_rendered_state = false;

  for (;;) {
    UiState state{};
    if (xQueueReceive(ui_state_queue_, &state, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    if (!has_rendered_state || !same_state(state, last_rendered_state)) {
      render_state(state);
      last_rendered_state = state;
      has_rendered_state = true;
    }
  }
}

void App::diagnostics_task_loop() {
  for (;;) {
    const AppState state = snapshot_state();

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
