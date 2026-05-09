/**
 * Screen composition helpers for the native sandbox app.
 *
 * Features (EN):
 * - Builds the small set of sandbox screens from runtime state.
 * - Keeps visual strings and screen layout in one place.
 * - Makes the app controller smaller and easier to scan.
 *
 * Funkcje (PL):
 * - Buduje niewielki zestaw ekranow sandboxa ze stanu runtime.
 * - Trzyma napisy i uklady ekranow w jednym miejscu.
 * - Upraszcza kontroler aplikacji i ulatwia jego przeglad.
 *
 * File: sandbox/espidf_native/main/src/native_screens.cpp
 */

#include "sandbox/native_screens.hpp"

#include <cstdio>

namespace sandbox {
namespace {
constexpr uint16_t k_color_white = 0xFFFF;
constexpr uint16_t k_color_cyan = 0x07FF;
constexpr uint16_t k_color_green = 0x07E0;
constexpr uint16_t k_color_yellow = 0xFFE0;

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
}  // namespace

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
}  // namespace sandbox
