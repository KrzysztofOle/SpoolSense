/**
 * Application orchestration layer for the task-based runtime.
 *
 * Features (EN):
 * - Owns firmware lifecycle startup and recovery.
 * - Starts the App, RFID, HX711, UI, and diagnostics tasks.
 * - Defines the queue-based message types shared between tasks.
 *
 * Funkcje (PL):
 * - Zarzadza startem i odzyskiwaniem cyklu zycia firmware.
 * - Uruchamia taski App, RFID, HX711, UI i diagnostyki.
 * - Definiuje typy komunikatow przekazywanych przez kolejki.
 *
 * File: components/app/include/app/app.hpp
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdint.h>

namespace app {
constexpr uint8_t kMaxRfidUidLength = 10;

enum class RfidStatus : uint8_t {
  kBooting,
  kReaderMissing,
  kWaitingForCard,
  kCardPresent,
  kCardRemoved,
};

enum class Hx711Status : uint8_t {
  kBooting,
  kNotFound,
  kReady,
};

struct RfidEvent {
  enum class Kind : uint8_t {
    kReaderMissing,
    kReaderReady,
    kWaitingForCard,
    kCardRemoved,
    kCardPresent,
  };

  Kind kind = Kind::kReaderMissing;
  uint32_t firmware_version = 0;
  bool has_uid = false;
  bool usage_available = false;
  bool page_read_failed = false;
  uint8_t uid[kMaxRfidUidLength] = {};
  uint8_t uid_length = 0;
  uint8_t usage_page[4] = {};
  uint32_t usage_seconds = 0;
  uint8_t life_percent = 0;
  uint32_t timestamp_ms = 0;
};

struct WeightEvent {
  enum class Kind : uint8_t {
    kNotFound,
    kReady,
    kSample,
  };

  Kind kind = Kind::kNotFound;
  bool has_sample = false;
  long raw_value = 0;
  uint32_t timestamp_ms = 0;
};

struct AppState {
  RfidStatus rfid_status = RfidStatus::kBooting;
  Hx711Status hx711_status = Hx711Status::kBooting;
  bool rfid_has_uid = false;
  bool rfid_usage_available = false;
  bool rfid_page_read_failed = false;
  bool hx711_has_sample = false;
  uint8_t rfid_uid[kMaxRfidUidLength] = {};
  uint8_t rfid_uid_length = 0;
  uint32_t rfid_firmware_version = 0;
  uint32_t rfid_usage_seconds = 0;
  uint8_t rfid_life_percent = 0;
  long hx711_raw_value = 0;
  uint32_t rfid_last_change_ms = 0;
  uint32_t hx711_last_sample_ms = 0;
};

struct UiState {
  RfidStatus rfid_status = RfidStatus::kBooting;
  Hx711Status hx711_status = Hx711Status::kBooting;
  bool rfid_has_uid = false;
  bool rfid_usage_available = false;
  bool rfid_page_read_failed = false;
  bool hx711_has_sample = false;
  uint8_t rfid_uid[kMaxRfidUidLength] = {};
  uint8_t rfid_uid_length = 0;
  uint32_t rfid_firmware_version = 0;
  uint32_t rfid_usage_seconds = 0;
  uint8_t rfid_life_percent = 0;
  long hx711_raw_value = 0;
  uint32_t rfid_last_change_ms = 0;
  uint32_t hx711_last_sample_ms = 0;
};

class App {
 public:
  App() = default;

  void begin();
  void restart();

 private:
  static constexpr UBaseType_t kRfidEventQueueLength = 8;
  static constexpr UBaseType_t kWeightEventQueueLength = 8;
  static constexpr UBaseType_t kUiStateQueueLength = 1;

  static void app_task_entry(void *arg);
  static void rfid_task_entry(void *arg);
  static void hx711_task_entry(void *arg);
  static void ui_task_entry(void *arg);
  static void diagnostics_task_entry(void *arg);

  void app_task_loop();
  void rfid_task_loop();
  void hx711_task_loop();
  void ui_task_loop();
  void diagnostics_task_loop();
  void start_tasks();
  void stop_tasks();
  void reset_state();
  void publish_state(const AppState &state);
  AppState snapshot_state() const;
  UiState snapshot_ui_state() const;
  void publish_ui_state(const UiState &state);
  bool publish_rfid_event(const RfidEvent &event);
  bool publish_weight_event(const WeightEvent &event);
  bool receive_rfid_event(RfidEvent *event);
  bool receive_weight_event(WeightEvent *event);
  void handle_rfid_event(const RfidEvent &event);
  void handle_weight_event(const WeightEvent &event);
  bool ui_state_changed(const UiState &lhs, const UiState &rhs) const;

  mutable SemaphoreHandle_t state_mutex_ = nullptr;
  TaskHandle_t app_task_handle_ = nullptr;
  TaskHandle_t rfid_task_handle_ = nullptr;
  TaskHandle_t hx711_task_handle_ = nullptr;
  TaskHandle_t ui_task_handle_ = nullptr;
  TaskHandle_t diagnostics_task_handle_ = nullptr;
  QueueHandle_t rfid_event_queue_ = nullptr;
  QueueHandle_t weight_event_queue_ = nullptr;
  QueueHandle_t ui_state_queue_ = nullptr;
  AppState app_state_{};
};
}  // namespace app
