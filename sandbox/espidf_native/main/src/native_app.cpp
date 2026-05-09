/**
 * Native ESP-IDF sandbox application controller.
 *
 * Features (EN):
 * - Coordinates LCD output, button input, screen selection, and counters.
 * - Keeps the sandbox app modular while staying free of Arduino dependencies.
 * - Renders a simple navigation flow with animation and counters.
 *
 * Funkcje (PL):
 * - Koordynuje LCD, wejscie z przyciskow, wybor ekranow i liczniki.
 * - Utrzymuje modularnosc sandboxa bez zaleznosci od Arduino.
 * - Rysuje prosty flow nawigacyjny z animacja i licznikami.
 *
 * File: sandbox/espidf_native/main/src/native_app.cpp
 */

#include "sandbox/native_app.hpp"

#include <array>
#include <cstdint>
#include <cstdio>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sandbox/native_lcd.hpp"
#include "sandbox/native_renderer.hpp"

namespace sandbox {
namespace {
constexpr gpio_num_t k_btn_a_pin = GPIO_NUM_39;
constexpr gpio_num_t k_btn_b_pin = GPIO_NUM_38;
constexpr gpio_num_t k_btn_c_pin = GPIO_NUM_37;

constexpr uint16_t k_color_white = 0xFFFF;
constexpr uint16_t k_color_cyan = 0x07FF;
constexpr uint16_t k_color_green = 0x07E0;
constexpr uint16_t k_color_yellow = 0xFFE0;

enum class ScreenMode : uint8_t {
  kWelcome,
  kStatus,
  kButtons,
  kMenu,
};

struct ButtonSnapshot {
  bool a = false;
  bool b = false;
  bool c = false;
};

struct ButtonCounters {
  uint32_t a = 0;
  uint32_t b = 0;
  uint32_t c = 0;
};

uint32_t millis_now() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

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

std::array<RenderLine, 4> welcome_lines() {
  return {{
      {"ESP-IDF NATIVE", 56, k_color_white},
      {"NO ARDUINO", 100, k_color_cyan},
      {"M5STACK CORE", 144, k_color_green},
      {"BTN A/B/C NAV", 188, k_color_yellow},
  }};
}

std::array<RenderLine, 4> status_lines(uint32_t uptime_ms, uint32_t frame_count, const ButtonSnapshot &buttons) {
  static char line1[32] = {};
  std::snprintf(line1, sizeof(line1), "UPTIME %lus", static_cast<unsigned long>(uptime_ms / 1000U));

  static char line2[32] = {};
  std::snprintf(line2, sizeof(line2), "FRAME %04lu", static_cast<unsigned long>(frame_count % 10000U));

  static char line3[32] = {};
  std::snprintf(line3, sizeof(line3), "BTN %u %u %u", buttons.a ? 1U : 0U, buttons.b ? 1U : 0U,
                buttons.c ? 1U : 0U);

  return {{
      {line1, 56, k_color_white},
      {line2, 100, k_color_cyan},
      {line3, 144, k_color_green},
      {"ESP-IDF ONLY", 188, k_color_yellow},
  }};
}

std::array<RenderLine, 4> buttons_lines(const ButtonSnapshot &buttons) {
  return {{
      {"BUTTON TEST", 56, k_color_white},
      {buttons.a ? "A PRESSED" : "A RELEASED", 100, k_color_cyan},
      {buttons.b ? "B PRESSED" : "B RELEASED", 144, k_color_green},
      {buttons.c ? "C PRESSED" : "C RELEASED", 188, k_color_yellow},
  }};
}

std::array<RenderLine, 4> menu_lines(const ButtonCounters &counters) {
  static char line1[32] = {};
  std::snprintf(line1, sizeof(line1), "MENU");

  static char line2[32] = {};
  std::snprintf(line2, sizeof(line2), "CNT %lu %lu %lu", static_cast<unsigned long>(counters.a),
                static_cast<unsigned long>(counters.b), static_cast<unsigned long>(counters.c));

  static char line3[32] = {};
  std::snprintf(line3, sizeof(line3), "A WELCOME");

  static char line4[32] = {};
  std::snprintf(line4, sizeof(line4), "B STATUS / C BUTTONS");

  return {{
      {line1, 44, k_color_white},
      {line2, 84, k_color_cyan},
      {line3, 124, k_color_green},
      {line4, 164, k_color_yellow},
  }};
}

const char *screen_name(ScreenMode screen) {
  switch (screen) {
    case ScreenMode::kWelcome:
      return "welcome";
    case ScreenMode::kStatus:
      return "status";
    case ScreenMode::kButtons:
      return "buttons";
    case ScreenMode::kMenu:
      return "menu";
  }

  return "unknown";
}

std::array<RenderLine, 4> build_lines(ScreenMode screen, uint32_t uptime_ms, uint32_t frame_count,
                                      const ButtonSnapshot &buttons, const ButtonCounters &counters) {
  switch (screen) {
    case ScreenMode::kWelcome:
      return welcome_lines();
    case ScreenMode::kStatus:
      return status_lines(uptime_ms, frame_count, buttons);
    case ScreenMode::kButtons:
      return buttons_lines(buttons);
    case ScreenMode::kMenu:
      return menu_lines(counters);
  }

  return welcome_lines();
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
