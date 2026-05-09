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
#include "sandbox/native_renderer.hpp"
#include "sandbox/native_screens.hpp"

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

  ScreenMode current_screen = ScreenMode::kWelcome;
  ButtonSnapshot last_buttons{};
  bool have_last_buttons = false;
  ButtonCounters click_counters{};
  uint32_t boot_ms = millis_now();
  uint32_t frame_count = 0;
  uint32_t last_rendered_animation_tick = UINT32_MAX;
  bool dirty = true;

  std::array<uint16_t, NativeLcd::kWidth> line{};

  for (;;) {
    const uint32_t now_ms = millis_now();
    const uint32_t elapsed_ms = now_ms - boot_ms;
    const uint32_t animation_tick = elapsed_ms / 120U;

    const ButtonSnapshot buttons = read_buttons();
    if (!have_last_buttons || buttons.a != last_buttons.a || buttons.b != last_buttons.b ||
        buttons.c != last_buttons.c) {
      if (is_pressed_edge(buttons.a, last_buttons.a)) {
        ++click_counters.a;
        current_screen = ScreenMode::kWelcome;
        dirty = true;
      }
      if (is_pressed_edge(buttons.b, last_buttons.b)) {
        ++click_counters.b;
        current_screen = ScreenMode::kStatus;
        dirty = true;
      }
      if (is_pressed_edge(buttons.c, last_buttons.c)) {
        ++click_counters.c;
        current_screen = ScreenMode::kButtons;
        dirty = true;
      }
      if (buttons.a && buttons.b && !last_buttons.a && !last_buttons.b) {
        current_screen = ScreenMode::kMenu;
        dirty = true;
      }
      last_buttons = buttons;
      have_last_buttons = true;
    }

    if (dirty || animation_tick != last_rendered_animation_tick) {
      const std::array<RenderLine, 4> lines =
          build_lines(current_screen, elapsed_ms, frame_count, buttons, click_counters);
      ESP_LOGI("sandbox", "render screen=%s uptime=%lu frame=%lu", screen_name(current_screen),
               static_cast<unsigned long>(elapsed_ms / 1000U), static_cast<unsigned long>(frame_count));

      for (int y = 0; y < NativeLcd::kHeight; ++y) {
        render_scanline(lines, y, animation_tick, line.data());
        if (lcd.send_line(y, line.data()) != ESP_OK) {
          ESP_LOGE("sandbox", "LCD transfer failed at line %d", y);
          break;
        }
        if (!lcd.wait_tx_done()) {
          ESP_LOGE("sandbox", "LCD transfer timeout at line %d", y);
          break;
        }
      }

      last_rendered_animation_tick = animation_tick;
      dirty = false;
      ++frame_count;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
}  // namespace sandbox
