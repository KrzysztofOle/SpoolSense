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
 * File: components/hx711/include/hx711/hx711_monitor.hpp
 */

#pragma once

#include <HX711.h>
#include <stdint.h>

class Hx711Monitor {
 public:
  Hx711Monitor();

  void begin();
  void update();

 private:
  HX711 scale_;
  bool scale_initialized_;
  bool scale_ready_;
  uint32_t next_hx711_sample_at_ms_;
  uint32_t next_hx711_init_retry_at_ms_;
};
