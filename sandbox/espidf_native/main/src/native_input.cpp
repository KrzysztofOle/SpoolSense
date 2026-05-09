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
 * File: sandbox/espidf_native/main/src/native_input.cpp
 */

#include "sandbox/native_input.hpp"

#include "driver/gpio.h"

namespace sandbox {
namespace {
constexpr gpio_num_t k_btn_a_pin = GPIO_NUM_39;
constexpr gpio_num_t k_btn_b_pin = GPIO_NUM_38;
constexpr gpio_num_t k_btn_c_pin = GPIO_NUM_37;
}  // namespace

void configure_buttons() {
  gpio_config_t io_config = {};
  io_config.intr_type = GPIO_INTR_DISABLE;
  io_config.mode = GPIO_MODE_INPUT;
  io_config.pin_bit_mask = (1ULL << k_btn_a_pin) | (1ULL << k_btn_b_pin) | (1ULL << k_btn_c_pin);
  io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_config.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_config);
}

ButtonSnapshot read_buttons() {
  ButtonSnapshot snapshot{};
  snapshot.a = gpio_get_level(k_btn_a_pin) == 0;
  snapshot.b = gpio_get_level(k_btn_b_pin) == 0;
  snapshot.c = gpio_get_level(k_btn_c_pin) == 0;
  return snapshot;
}

bool is_pressed_edge(bool current, bool previous) {
  return current && !previous;
}
}  // namespace sandbox
