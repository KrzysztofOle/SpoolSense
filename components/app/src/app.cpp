/**
 * Application orchestration layer.
 *
 * Features (EN):
 * - Owns the firmware lifecycle, FreeRTOS task startup, and queue handling.
 * - Routes RFID and optional HX711 events through queues and publishes UI snapshots.
 *
 * Funkcje (PL):
 * - Zarzadza cyklem zycia firmware i startem taskow FreeRTOS.
 * - Przekierowuje zdarzenia RFID i opcjonalnego HX711 przez kolejki oraz publikuje snapshoty UI.
 *
 * File: components/app/src/app.cpp
 */

#include "app/app.hpp"

#include <Arduino.h>
#include <M5Unified.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <sdkconfig.h>

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
constexpr uint32_t kHx711ZeroSamples = 8;
constexpr uint32_t kHx711MissingTimeoutMs = 500;
constexpr float kHx711CountsPerGram = 1000.0f;
constexpr uint32_t kDiagnosticsIntervalMs = 5000;
constexpr uint32_t kAppStartupTimeoutMs = 5000;

bool same_uid(const uint8_t *lhs, const uint8_t *rhs, uint8_t uid_length) {
  if (uid_length == 0) {
    return false;
  }

  return std::memcmp(lhs, rhs, uid_length) == 0;
}

void clear_uid(uint8_t *uid) {
  std::memset(uid, 0, kMaxRfidUidLength);
}

int32_t raw_to_grams(long raw_value) {
  return static_cast<int32_t>(std::lround(static_cast<double>(raw_value) / kHx711CountsPerGram));
}

UiState to_ui_state(const AppState &state) {
  UiState ui_state{};
  ui_state.current_mode = state.current_mode;
  ui_state.rfid_status = state.rfid_status;
  ui_state.hx711_status = state.hx711_status;
  ui_state.hx711_enabled = state.hx711_enabled;
  ui_state.rfid_has_uid = state.rfid_has_uid;
  ui_state.rfid_usage_available = state.rfid_usage_available;
  ui_state.rfid_page_read_failed = state.rfid_page_read_failed;
  ui_state.hx711_has_sample = state.hx711_has_sample;
  ui_state.rfid_uid_length = state.rfid_uid_length;
  ui_state.rfid_firmware_version = state.rfid_firmware_version;
  ui_state.rfid_usage_seconds = state.rfid_usage_seconds;
  ui_state.rfid_life_percent = state.rfid_life_percent;
  ui_state.hx711_raw_value = state.hx711_raw_value;
  ui_state.hx711_weight_grams = state.hx711_weight_grams;
  ui_state.rfid_last_change_ms = state.rfid_last_change_ms;
  ui_state.hx711_last_sample_ms = state.hx711_last_sample_ms;
  ui_state.hx711_zeroed = state.hx711_zeroed;
  ui_state.hx711_zeroed_ms = state.hx711_zeroed_ms;
  ui_state.last_button = state.last_button;
  ui_state.last_button_ms = state.last_button_ms;
  if (state.rfid_has_uid) {
    std::memcpy(ui_state.rfid_uid, state.rfid_uid, sizeof(ui_state.rfid_uid));
  }
  return ui_state;
}

bool same_state(const UiState &lhs, const UiState &rhs) {
  return lhs.current_mode == rhs.current_mode && lhs.rfid_status == rhs.rfid_status &&
         lhs.hx711_status == rhs.hx711_status && lhs.hx711_enabled == rhs.hx711_enabled &&
         lhs.rfid_has_uid == rhs.rfid_has_uid &&
         lhs.rfid_usage_available == rhs.rfid_usage_available &&
         lhs.rfid_page_read_failed == rhs.rfid_page_read_failed &&
         lhs.hx711_has_sample == rhs.hx711_has_sample &&
         lhs.rfid_uid_length == rhs.rfid_uid_length &&
         lhs.rfid_firmware_version == rhs.rfid_firmware_version &&
         lhs.rfid_usage_seconds == rhs.rfid_usage_seconds &&
         lhs.rfid_life_percent == rhs.rfid_life_percent &&
         lhs.hx711_raw_value == rhs.hx711_raw_value &&
         lhs.hx711_weight_grams == rhs.hx711_weight_grams &&
         lhs.rfid_last_change_ms == rhs.rfid_last_change_ms &&
         lhs.hx711_last_sample_ms == rhs.hx711_last_sample_ms &&
         lhs.hx711_zeroed == rhs.hx711_zeroed &&
         lhs.hx711_zeroed_ms == rhs.hx711_zeroed_ms &&
         lhs.last_button == rhs.last_button &&
         lhs.last_button_ms == rhs.last_button_ms &&
         std::memcmp(lhs.rfid_uid, rhs.rfid_uid, sizeof(lhs.rfid_uid)) == 0;
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
    case Hx711Status::kDisabled:
      return "HX711: disabled";
    case Hx711Status::kNotFound:
      return "HX711: missing";
    case Hx711Status::kReady:
      return "HX711: ready";
  }

  return "HX711: unknown";
}

const char *button_text(ButtonKind kind) {
  switch (kind) {
    case ButtonKind::kA:
      return "BtnA";
    case ButtonKind::kB:
      return "BtnB";
    case ButtonKind::kC:
      return "BtnC";
    case ButtonKind::kNone:
      return "none";
  }

  return "unknown";
}

const char *app_mode_text(AppMode mode) {
  switch (mode) {
    case AppMode::kBoot:
      return "Mode: boot";
    case AppMode::kIdle:
      return "Mode: idle";
    case AppMode::kTagDetected:
      return "Mode: tag detected";
    case AppMode::kMeasuring:
      return "Mode: measuring";
    case AppMode::kError:
      return "Mode: error";
    case AppMode::kCalibration:
      return "Mode: calibration";
  }

  return "Mode: unknown";
}

void log_mode_if_changed(AppMode before_mode, AppMode after_mode) {
  if (before_mode != after_mode) {
    diagnostics::log_line(app_mode_text(after_mode));
  }
}

void log_button_event_effects(const AppState &after_state, const ButtonEvent &event) {
  if (event.kind == ButtonKind::kNone) {
    return;
  }

  char line[32] = {};
  std::snprintf(line, sizeof(line), "%s clicked", button_text(event.kind));
  diagnostics::log_line(line);

  if (after_state.last_button != ButtonKind::kNone) {
    std::snprintf(line, sizeof(line), "Last button: %s", button_text(after_state.last_button));
    diagnostics::log_line(line);
  }
}

void log_rfid_event_effects(const AppState &before_state, const AppState &after_state, const RfidEvent &event) {
  switch (event.kind) {
    case RfidEvent::Kind::kReaderMissing:
      diagnostics::log_line("PN532 not found");
      break;

    case RfidEvent::Kind::kReaderReady: {
      char version_line[40] = {};
      std::snprintf(version_line, sizeof(version_line), "PN532 FW: 0x%08lX",
                    static_cast<unsigned long>(event.firmware_version));
      diagnostics::log_line(version_line);
      diagnostics::log_line("PN532 init OK");
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.hx711_status != Hx711Status::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;
    }

    case RfidEvent::Kind::kWaitingForCard:
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.hx711_status != Hx711Status::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;

    case RfidEvent::Kind::kCardRemoved:
      diagnostics::log_line("Card removed");
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.hx711_status != Hx711Status::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;

    case RfidEvent::Kind::kCardPresent:
      if (event.usage_available) {
        diagnostics::log_uid_and_usage(event.uid, event.uid_length, event.usage_page, event.usage_seconds,
                                       event.life_percent);
      } else {
        diagnostics::log_uid_and_type(event.uid, event.uid_length);
        if (event.page_read_failed) {
          diagnostics::log_line("Page 24: read failed");
        }
      }
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.hx711_status != Hx711Status::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;
  }

  log_mode_if_changed(before_state.current_mode, after_state.current_mode);
}

void log_weight_event_effects(const AppState &before_state, const AppState &after_state, const WeightEvent &event) {
  if (!before_state.hx711_enabled) {
    return;
  }

  switch (event.kind) {
    case WeightEvent::Kind::kNotFound:
      diagnostics::log_line("HX711 not found");
      break;

    case WeightEvent::Kind::kReady:
      diagnostics::log_line("HX711 init OK");
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.rfid_status != RfidStatus::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;

    case WeightEvent::Kind::kSample:
      if (event.has_sample) {
        diagnostics::log_weight_grams(event.raw_value, event.weight_grams);
      }
      if (after_state.current_mode == AppMode::kError && before_state.current_mode != AppMode::kError &&
          before_state.rfid_status != RfidStatus::kBooting) {
        diagnostics::log_line("Sensor fault");
      }
      break;

    case WeightEvent::Kind::kZeroed:
      diagnostics::log_hx711_zeroed();
      break;
  }

  log_mode_if_changed(before_state.current_mode, after_state.current_mode);
}

void render_developer_screen(const UiState &state) {
  char line1[48] = {};
  std::snprintf(line1, sizeof(line1), "%s", app_mode_text(state.current_mode));

  char line2[48] = {};
  std::snprintf(line2, sizeof(line2), "%s", rfid_status_text(state.rfid_status));

  char line3[48] = {};
  std::snprintf(line3, sizeof(line3), "%s", hx711_status_text(state.hx711_status));

  char line4[96] = {};
  if (state.hx711_has_sample) {
    std::snprintf(line4, sizeof(line4), "Weight: %ld g", static_cast<long>(state.hx711_weight_grams));
  } else if (state.hx711_zeroed) {
    std::snprintf(line4, sizeof(line4), "HX711 zeroed");
  } else if (state.rfid_has_uid) {
    char uid_line[48] = {};
    diagnostics::format_uid(state.rfid_uid, state.rfid_uid_length, uid_line, sizeof(uid_line));
    size_t used = 0;
    used += static_cast<size_t>(std::snprintf(line4 + used, sizeof(line4) - used, "UID: %s", uid_line));
    if (state.last_button != ButtonKind::kNone) {
      if (used + 1 < sizeof(line4)) {
        (void)std::snprintf(line4 + used, sizeof(line4) - used, " Btn: %s", button_text(state.last_button));
      }
    }
  } else if (state.last_button != ButtonKind::kNone) {
    std::snprintf(line4, sizeof(line4), "Btn: %s", button_text(state.last_button));
  } else if (state.hx711_enabled) {
    std::snprintf(line4, sizeof(line4), "RFID/UI ready");
  } else {
    std::snprintf(line4, sizeof(line4), "HX711 disabled");
  }

  display::show_text(line1, line2);
  display::append_line(line3);
  display::append_line(line4);
}

void log_timeout_effects(const AppState &before_state, const AppState &after_state, uint32_t now_ms) {
  if (after_state.current_mode == before_state.current_mode) {
    return;
  }

  if (before_state.rfid_status == RfidStatus::kBooting &&
      now_ms - before_state.rfid_last_change_ms > kAppStartupTimeoutMs) {
    diagnostics::log_line("PN532 init timeout");
  } else if (before_state.hx711_status == Hx711Status::kBooting &&
             now_ms - before_state.hx711_last_sample_ms > kAppStartupTimeoutMs) {
    diagnostics::log_line("HX711 init timeout");
  }

  log_mode_if_changed(before_state.current_mode, after_state.current_mode);
}

void render_state(const UiState &state) {
  render_developer_screen(state);
}
}  // namespace

void App::begin() {
  Serial.begin(115200);
  auto cfg = M5.config();
  cfg.fallback_board = m5::board_t::board_M5Stack;
  M5.begin(cfg);
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

#if defined(CONFIG_SPOOLSENSE_ENABLE_HX711) && CONFIG_SPOOLSENSE_ENABLE_HX711
  if (weight_event_queue_ == nullptr) {
    weight_event_queue_ = xQueueCreate(kWeightEventQueueLength, sizeof(WeightEvent));
  } else {
    xQueueReset(weight_event_queue_);
  }
  if (hx711_command_queue_ == nullptr) {
    hx711_command_queue_ = xQueueCreate(kHx711CommandQueueLength, sizeof(Hx711Command));
  } else {
    xQueueReset(hx711_command_queue_);
  }
#else
  if (weight_event_queue_ != nullptr) {
    xQueueReset(weight_event_queue_);
  }
  if (hx711_command_queue_ != nullptr) {
    xQueueReset(hx711_command_queue_);
  }
#endif

  if (ui_state_queue_ == nullptr) {
    ui_state_queue_ = xQueueCreate(kUiStateQueueLength, sizeof(UiState));
  } else {
    xQueueReset(ui_state_queue_);
  }

  if (rfid_event_queue_ == nullptr || ui_state_queue_ == nullptr) {
    diagnostics::log_line("Queue allocation failed");
    stop_tasks();
    return;
  }

#if defined(CONFIG_SPOOLSENSE_ENABLE_HX711) && CONFIG_SPOOLSENSE_ENABLE_HX711
  if (weight_event_queue_ == nullptr) {
    diagnostics::log_line("HX711 queue allocation failed");
    stop_tasks();
    return;
  }
  if (hx711_command_queue_ == nullptr) {
    diagnostics::log_line("HX711 command queue allocation failed");
    stop_tasks();
    return;
  }
#endif

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

#if defined(CONFIG_SPOOLSENSE_ENABLE_HX711) && CONFIG_SPOOLSENSE_ENABLE_HX711
  result = xTaskCreate(hx711_task_entry, "hx711_task", 4096, this, 2, &hx711_task_handle_);
  if (result != pdPASS) {
    diagnostics::log_line("HX711 task start failed");
    stop_tasks();
    return;
  }
#else
  diagnostics::log_line("HX711 disabled");
#endif

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

#if defined(CONFIG_SPOOLSENSE_ENABLE_HX711) && CONFIG_SPOOLSENSE_ENABLE_HX711
  if (hx711_task_handle_ != nullptr) {
    vTaskDelete(hx711_task_handle_);
    hx711_task_handle_ = nullptr;
  }
#endif

  if (ui_task_handle_ != nullptr) {
    vTaskDelete(ui_task_handle_);
    ui_task_handle_ = nullptr;
  }

  if (diagnostics_task_handle_ != nullptr) {
    vTaskDelete(diagnostics_task_handle_);
    diagnostics_task_handle_ = nullptr;
  }

  if (hx711_command_queue_ != nullptr) {
    xQueueReset(hx711_command_queue_);
  }
}

void App::reset_state() {
  if (state_mutex_ == nullptr) {
    state_mutex_ = xSemaphoreCreateMutex();
  }

  AppState state{};
  fsm_.reset(state, millis());
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

bool App::publish_hx711_command(const Hx711Command &command) {
  if (hx711_command_queue_ == nullptr) {
    return false;
  }

  return xQueueSendToBack(hx711_command_queue_, &command, 0) == pdTRUE;
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

bool App::receive_hx711_command(Hx711Command *command) {
  if (command == nullptr || hx711_command_queue_ == nullptr) {
    return false;
  }

  return xQueueReceive(hx711_command_queue_, command, 0) == pdTRUE;
}

bool App::ui_state_changed(const UiState &lhs, const UiState &rhs) const {
  return !same_state(lhs, rhs);
}

void App::handle_button_input(AppState &state, bool &handled_event) {
  M5.update();

  const uint32_t now = millis();
  const struct {
    ButtonKind kind;
    bool clicked;
  } button_events[] = {
      {ButtonKind::kA, M5.BtnA.wasClicked()},
      {ButtonKind::kB, M5.BtnB.wasClicked()},
      {ButtonKind::kC, M5.BtnC.wasClicked()},
  };

  for (const auto &button_event : button_events) {
    if (!button_event.clicked) {
      continue;
    }

    ButtonEvent event{};
    event.kind = button_event.kind;
    event.timestamp_ms = now;

    const AppState before_state = state;
    fsm_.handle_event(state, event);
    log_button_event_effects(state, event);

    if (button_event.kind == ButtonKind::kA && state.hx711_enabled) {
      Hx711Command command{};
      command.kind = Hx711Command::Kind::kZero;
      command.timestamp_ms = now;
      if (!publish_hx711_command(command)) {
        diagnostics::log_line("HX711 zero request dropped");
      }
    }

    if (before_state.last_button != state.last_button || before_state.last_button_ms != state.last_button_ms) {
      handled_event = true;
    }
  }
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
  AppState state = snapshot_state();
  UiState last_published_state = to_ui_state(state);

  for (;;) {
    bool handled_event = false;

    handle_button_input(state, handled_event);

    RfidEvent rfid_event{};
    while (receive_rfid_event(&rfid_event)) {
      const AppState before_state = state;
      fsm_.handle_event(state, rfid_event);
      log_rfid_event_effects(before_state, state, rfid_event);
      handled_event = true;
    }

    WeightEvent weight_event{};
    while (receive_weight_event(&weight_event)) {
      const AppState before_state = state;
      fsm_.handle_event(state, weight_event);
      log_weight_event_effects(before_state, state, weight_event);
      handled_event = true;
    }

    const uint32_t now = millis();
    const AppState before_timeout_state = state;
    if (fsm_.handle_timeouts(state, now)) {
      log_timeout_effects(before_timeout_state, state, now);
      handled_event = true;
    }

    if (handled_event) {
      publish_state(state);
      const UiState current_state = to_ui_state(state);
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
  bool zero_pending = false;
  uint32_t missing_since_ms = 0;

  monitor.begin();

  for (;;) {
    const uint32_t now = millis();

    Hx711Command command{};
    while (receive_hx711_command(&command)) {
      if (command.kind == Hx711Command::Kind::kZero) {
        zero_pending = true;
      }
    }

    if (!monitor.is_ready()) {
      if (missing_since_ms == 0) {
        missing_since_ms = now;
      }

      if (now - missing_since_ms < kHx711MissingTimeoutMs) {
        vTaskDelay(pdMS_TO_TICKS(25));
        continue;
      }

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
      missing_since_ms = 0;
      next_sample_at_ms = 0;
      next_init_retry_at_ms = now + kHx711InitRetryMs;
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (!ready_reported || zero_pending) {
      if (!monitor.zero(kHx711ZeroSamples)) {
        if (missing_since_ms == 0) {
          missing_since_ms = now;
        }

        if (now - missing_since_ms >= kHx711MissingTimeoutMs) {
          WeightEvent event{};
          event.kind = WeightEvent::Kind::kNotFound;
          event.timestamp_ms = now;
          if (!publish_weight_event(event)) {
            vTaskDelay(pdMS_TO_TICKS(50));
          }
          ready_reported = false;
          missing_since_ms = 0;
          next_init_retry_at_ms = now + kHx711InitRetryMs;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
        continue;
      }

      WeightEvent event{};
      event.kind = zero_pending ? WeightEvent::Kind::kZeroed : WeightEvent::Kind::kReady;
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      ready_reported = true;
      missing_since_ms = 0;
      zero_pending = false;
      next_sample_at_ms = now + kHx711SampleIntervalMs;
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
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
      missing_since_ms = 0;
      WeightEvent event{};
      event.kind = WeightEvent::Kind::kSample;
      event.has_sample = true;
      event.raw_value = raw_value;
      event.weight_grams = raw_to_grams(raw_value);
      event.timestamp_ms = now;
      if (!publish_weight_event(event)) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      next_sample_at_ms = now + kHx711SampleIntervalMs;
    } else {
      if (missing_since_ms == 0) {
        missing_since_ms = now;
      }

      if (now - missing_since_ms < kHx711MissingTimeoutMs) {
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
      zero_pending = false;
      missing_since_ms = 0;
      next_sample_at_ms = 0;
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
    diagnostics::log_line(app_mode_text(state.current_mode));
    diagnostics::log_line(rfid_status_text(state.rfid_status));
    diagnostics::log_line(hx711_status_text(state.hx711_status));
    if (!state.hx711_enabled) {
      diagnostics::log_line("RFID/UI developer mode");
    }

    if (state.rfid_has_uid) {
      diagnostics::log_uid_and_type(state.rfid_uid, state.rfid_uid_length);
    }

    if (state.rfid_usage_available) {
      diagnostics::log_line("RFID usage available");
    }

    if (state.hx711_has_sample) {
      diagnostics::log_raw_weight(state.hx711_raw_value);
    }

    if (state.last_button != ButtonKind::kNone) {
      char button_line[32] = {};
      std::snprintf(button_line, sizeof(button_line), "Last button: %s", button_text(state.last_button));
      diagnostics::log_line(button_line);
    }

    vTaskDelay(pdMS_TO_TICKS(kDiagnosticsIntervalMs));
  }
}
}  // namespace app
