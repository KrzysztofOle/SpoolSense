/**
 * Application FSM extracted from runtime orchestration.
 *
 * Features (EN):
 * - Applies RFID and HX711 events to AppState.
 * - Handles transition logic and timeout transitions without runtime coupling.
 * - Keeps FSM testable without FreeRTOS or hardware dependencies.
 *
 * Funkcje (PL):
 * - Aplikuje zdarzenia RFID i HX711 do AppState.
 * - Obsluguje logike przejsc i timeouty bez sprzezenia z runtime.
 * - Pozwala testowac FSM bez FreeRTOS i zaleznosci sprzetowych.
 *
 * File: components/app/include/app/app_fsm.hpp
 */

#pragma once

#include <stdint.h>

#include "app/app_events.hpp"
#include "app/app_state.hpp"

namespace app {
class AppFsm {
 public:
  AppFsm() = default;

  void reset(AppState &state, uint32_t now_ms) const;
  bool handle_event(AppState &state, const RfidEvent &event) const;
  bool handle_event(AppState &state, const WeightEvent &event) const;
  bool handle_timeouts(AppState &state, uint32_t now_ms) const;

 private:
  static bool transition_to(AppState &state, AppMode next_mode);
  static void sync_mode(AppState &state);
};
}  // namespace app
