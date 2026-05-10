/**
 * Native ESP-IDF sandbox application controller.
 *
 * Features (EN):
 * - Coordinates LCD startup, button input, and screen updates.
 * - Keeps the sandbox app modular while staying free of Arduino dependencies.
 * - Focuses on runtime flow instead of rendering details.
 *
 * Funkcje (PL):
 * - Koordynuje start LCD, wejscie z przyciskow i odswiezanie ekranow.
 * - Utrzymuje modularnosc sandboxa bez zaleznosci od Arduino.
 * - Skupia sie na przeplywie runtime zamiast na szczegolach renderowania.
 *
 * File: sandbox/espidf_native/main/src/native_app.cpp
 */

#include "sandbox/native_app.hpp"

#include <array>
#include <cstdint>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sandbox/native_input.hpp"
#include "sandbox/native_lcd.hpp"
#include "sandbox/native_lvgl_ui.hpp"

namespace sandbox {
namespace {
uint32_t millis_now() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}
}  // namespace

void NativeApp::run() {
  ESP_LOGI("sandbox", "ESP-IDF native sandbox start");

  configure_buttons();

  NativeLcd lcd;
  if (lcd.begin() != ESP_OK) {
    ESP_LOGE("sandbox", "LCD init failed");
    return;
  }

  NativeLvglUi ui;
  if (!ui.begin(&lcd)) {
    ESP_LOGE("sandbox", "LVGL init failed");
    return;
  }

  ButtonSnapshot last_buttons{};
  bool have_last_buttons = false;
  ButtonCounters click_counters{};
  uint32_t boot_ms = millis_now();
  uint32_t last_loop_ms = boot_ms;
  uint32_t last_log_ms = boot_ms;
  uint32_t last_a_action_ms = boot_ms;
  uint32_t last_b_action_ms = boot_ms;
  uint32_t last_c_action_ms = boot_ms;
  constexpr uint32_t k_button_debounce_ms = 250U;

  for (;;) {
    const uint32_t now_ms = millis_now();
    const uint32_t elapsed_ms = now_ms - boot_ms;
    const uint32_t delta_ms = now_ms - last_loop_ms;
    last_loop_ms = now_ms;

    const ButtonSnapshot buttons = read_buttons();
    if (!have_last_buttons || buttons.a != last_buttons.a || buttons.b != last_buttons.b ||
        buttons.c != last_buttons.c) {
      if (is_pressed_edge(buttons.a, last_buttons.a)) {
        if ((now_ms - last_a_action_ms) >= k_button_debounce_ms) {
          last_a_action_ms = now_ms;
          ++click_counters.a;
          ui.next_pattern();
        }
      }
      if (is_pressed_edge(buttons.b, last_buttons.b)) {
        if ((now_ms - last_b_action_ms) >= k_button_debounce_ms) {
          last_b_action_ms = now_ms;
          ++click_counters.b;
          if (!ui.toggle_color_order()) {
            ESP_LOGE("sandbox", "Color order toggle failed");
          }
        }
      }
      if (is_pressed_edge(buttons.c, last_buttons.c)) {
        if ((now_ms - last_c_action_ms) >= k_button_debounce_ms) {
          last_c_action_ms = now_ms;
          ++click_counters.c;
          ui.reset_pattern();
        }
      }
      last_buttons = buttons;
      have_last_buttons = true;
    }

    if ((now_ms - last_log_ms) >= 2000U) {
      ESP_LOGI("sandbox", "lvgl uptime=%lus A:%lu B:%lu C:%lu", static_cast<unsigned long>(elapsed_ms / 1000U),
               static_cast<unsigned long>(click_counters.a), static_cast<unsigned long>(click_counters.b),
               static_cast<unsigned long>(click_counters.c));
      last_log_ms = now_ms;
    }

    ui.tick(delta_ms);
    ui.update(buttons, click_counters, elapsed_ms);
    ui.process();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
}  // namespace sandbox
