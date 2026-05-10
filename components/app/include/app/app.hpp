/**
 * Application runtime orchestration layer.
 *
 * Features (EN):
 * - Owns firmware lifecycle startup and shutdown.
 * - Starts the App, RFID, optional HX711, UI, and diagnostics tasks.
 * - Handles queues and runtime communication between components.
 *
 * Funkcje (PL):
 * - Zarzadza startem i zatrzymaniem cyklu zycia firmware.
 * - Uruchamia taski App, RFID, opcjonalny HX711, UI i diagnostyki.
 * - Obsluguje kolejki oraz komunikacje miedzy komponentami runtime.
 *
 * File: components/app/include/app/app.hpp
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdint.h>

#include "app/app_events.hpp"
#include "app/app_fsm.hpp"
#include "app/app_state.hpp"
#include "board/buttons.hpp"

namespace app {
class App {
 public:
  App() = default;

  void begin();
  void restart();

 private:
  static constexpr UBaseType_t kRfidEventQueueLength = 8;
  static constexpr UBaseType_t kWeightEventQueueLength = 8;
  static constexpr UBaseType_t kHx711CommandQueueLength = 4;
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
  void handle_button_input(AppState &state, bool &handled_event);
  void handle_home_input(AppState &state, const ButtonEvent &event);
  void handle_diagnostics_input(AppState &state, const ButtonEvent &event);
  void handle_scale_input(AppState &state, const ButtonEvent &event);
  void update_spool_metrics();
  void start_tasks();
  void stop_tasks();
  void reset_state();
  void publish_state(const AppState &state);
  AppState snapshot_state() const;
  UiState snapshot_ui_state() const;
  void publish_ui_state(const UiState &state);
  bool publish_rfid_event(const RfidEvent &event);
  bool publish_weight_event(const WeightEvent &event);
  bool publish_hx711_command(const Hx711Command &command);
  bool receive_rfid_event(RfidEvent *event);
  bool receive_weight_event(WeightEvent *event);
  bool receive_hx711_command(Hx711Command *command);
  bool ui_state_changed(const UiState &lhs, const UiState &rhs) const;

  AppFsm fsm_{};
  mutable SemaphoreHandle_t state_mutex_ = nullptr;
  TaskHandle_t app_task_handle_ = nullptr;
  TaskHandle_t rfid_task_handle_ = nullptr;
  TaskHandle_t hx711_task_handle_ = nullptr;
  TaskHandle_t ui_task_handle_ = nullptr;
  TaskHandle_t diagnostics_task_handle_ = nullptr;
  QueueHandle_t rfid_event_queue_ = nullptr;
  QueueHandle_t weight_event_queue_ = nullptr;
  QueueHandle_t hx711_command_queue_ = nullptr;
  QueueHandle_t ui_state_queue_ = nullptr;
  AppState app_state_{};
  HardwareAvailability hardware_{};
  board::ButtonController buttons_{};
};
}  // namespace app
