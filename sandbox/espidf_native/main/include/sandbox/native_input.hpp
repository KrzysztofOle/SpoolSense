/**
 * Native button input helpers for the sandbox app.
 *
 * Features (EN):
 * - Configures the classic M5Stack buttons using native GPIO.
 * - Reads button snapshots without Arduino dependencies.
 * - Detects press edges for simple navigation flows.
 *
 * Funkcje (PL):
 * - Konfiguruje klasyczne przyciski M5Stack przez natywne GPIO.
 * - Odczytuje stan przyciskow bez zaleznosci od Arduino.
 * - Wykrywa zbocza nacisniecia dla prostych przeplywow nawigacji.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_input.hpp
 */

#pragma once

namespace sandbox {
struct ButtonSnapshot {
  bool a = false;
  bool b = false;
  bool c = false;
};

void configure_buttons();
ButtonSnapshot read_buttons();
bool is_pressed_edge(bool current, bool previous);
}  // namespace sandbox
