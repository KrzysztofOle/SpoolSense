/**
 * HX711 access helper for the task-based runtime.
 *
 * Features (EN):
 * - Initializes the HX711 interface.
 * - Exposes direct readiness and raw sampling access for the HX711 task.
 *
 * Funkcje (PL):
 * - Inicjalizuje interfejs HX711.
 * - Udostepnia bezposredni dostep do gotowosci i probkowania dla taska HX711.
 *
 * File: components/hx711/include/hx711/hx711_monitor.hpp
 */

#pragma once

#include <HX711.h>
#include <stdint.h>

class Hx711Monitor {
 public:
  Hx711Monitor();

  void begin();
  bool is_ready() const;
  bool read_raw(long *raw_value);

 private:
  HX711 scale_;
};
