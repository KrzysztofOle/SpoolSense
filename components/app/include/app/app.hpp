/**
 * Application orchestration layer for the task-based runtime.
 *
 * Features (EN):
 * - Owns firmware lifecycle startup and recovery.
 * - Starts the RFID, HX711, UI, and diagnostics tasks.
 *
 * Funkcje (PL):
 * - Zarzadza startem i odzyskiwaniem cyklu zycia firmware.
 * - Uruchamia taski RFID, HX711, UI i diagnostyki.
 *
 * File: components/app/include/app/app.hpp
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdint.h>

namespace app {
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

class App {
 public:
  App() = default;

  void begin();
  void restart();

  struct RuntimeState {
    RfidStatus rfid_status = RfidStatus::kBooting;
    Hx711Status hx711_status = Hx711Status::kBooting;
    bool rfid_has_uid = false;
    bool rfid_usage_available = false;
    bool rfid_page_read_failed = false;
    bool hx711_has_sample = false;
    uint8_t rfid_uid[10] = {};
    uint8_t rfid_uid_length = 0;
    uint32_t rfid_firmware_version = 0;
    uint32_t rfid_usage_seconds = 0;
    uint8_t rfid_life_percent = 0;
    long hx711_raw_value = 0;
    uint32_t rfid_last_change_ms = 0;
    uint32_t hx711_last_sample_ms = 0;
  };

 private:
  static void rfid_task_entry(void *arg);
  static void hx711_task_entry(void *arg);
  static void ui_task_entry(void *arg);
  static void diagnostics_task_entry(void *arg);

  void rfid_task_loop();
  void hx711_task_loop();
  void ui_task_loop();
  void diagnostics_task_loop();
  void start_tasks();
  void stop_tasks();
  void reset_state();
  RuntimeState snapshot_state() const;
  void publish_state(const RuntimeState &state);
  void publish_rfid_state(const RuntimeState &state);
  void publish_hx711_state(const RuntimeState &state);

  mutable SemaphoreHandle_t state_mutex_ = nullptr;
  TaskHandle_t rfid_task_handle_ = nullptr;
  TaskHandle_t hx711_task_handle_ = nullptr;
  TaskHandle_t ui_task_handle_ = nullptr;
  TaskHandle_t diagnostics_task_handle_ = nullptr;
  RuntimeState runtime_state_{};
};
}  // namespace app
