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
#include "diagnostics/diagnostics.hpp"

namespace {
constexpr uint32_t kHx711SampleIntervalMs = 700;
constexpr uint32_t kHx711InitRetryMs = 1500;
}  // namespace

Hx711Monitor::Hx711Monitor()
    : scale_initialized_(false),
      scale_ready_(false),
      next_hx711_sample_at_ms_(0),
      next_hx711_init_retry_at_ms_(0) {}

void Hx711Monitor::begin() {
  scale_initialized_ = false;
  scale_ready_ = false;
  next_hx711_sample_at_ms_ = 0;
  next_hx711_init_retry_at_ms_ = 0;
}

void Hx711Monitor::update() {
  if (!scale_initialized_) {
    scale_.begin(board::kHx711DoutPin, board::kHx711SckPin);
    scale_initialized_ = true;
    next_hx711_init_retry_at_ms_ = 0;
  }

  const uint32_t now = millis();

  if (!scale_ready_) {
    if (now < next_hx711_init_retry_at_ms_) {
      delay(10);
      return;
    }

    if (scale_.is_ready()) {
      scale_ready_ = true;
      next_hx711_sample_at_ms_ = now;
      Serial.println("HX711 init OK");
    } else {
      Serial.println("HX711 not found");
      next_hx711_init_retry_at_ms_ = now + kHx711InitRetryMs;
      delay(100);
    }
    return;
  }

  if (now < next_hx711_sample_at_ms_) {
    delay(10);
    return;
  }

  if (scale_.is_ready()) {
    diagnostics::log_raw_weight(scale_.read());
    next_hx711_sample_at_ms_ = now + kHx711SampleIntervalMs;
  } else {
    scale_ready_ = false;
    next_hx711_init_retry_at_ms_ = now + 250;
    Serial.println("HX711 not found");
  }

  delay(20);
}
