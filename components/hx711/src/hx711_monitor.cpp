/**
 * HX711 polling helper for the current firmware flow.
 *
 * Features (EN):
 * - Keeps the existing HX711 initialization and sampling behavior.
 * - Exposes a small update loop for orchestration.
 *
 * Funkcje (PL):
 * - Zachowuje obecne zachowanie inicjalizacji i probkowania HX711.
 * - Udostepnia mala petle update dla orkiestracji.
 *
 * File: components/hx711/src/hx711_monitor.cpp
 */

#include "hx711/hx711_monitor.hpp"

#include <Arduino.h>

#include "board/board.hpp"

Hx711Monitor::Hx711Monitor() = default;

void Hx711Monitor::begin() {
  scale_.begin(board::kHx711DoutPin, board::kHx711SckPin);
}

bool Hx711Monitor::is_ready() const {
  return scale_.is_ready();
}

bool Hx711Monitor::read_raw(long *raw_value) {
  if (raw_value == nullptr || !scale_.is_ready()) {
    return false;
  }

  *raw_value = scale_.read();
  return true;
}
