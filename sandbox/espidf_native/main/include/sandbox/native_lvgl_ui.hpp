/**
 * Native LVGL UI layer for the sandbox app.
 *
 * Features (EN):
 * - Initializes LVGL with a native ESP-IDF LCD backend.
 * - Renders a dedicated LCD color test with large swatches.
 * - Exposes a lightweight update loop for the main runtime.
 *
 * Funkcje (PL):
 * - Inicjalizuje LVGL z natywnym backendem LCD dla ESP-IDF.
 * - Renderuje dedykowany test kolorow LCD z duzymi kwadratami.
 * - Udostepnia lekka petle aktualizacji dla glownego runtime.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_lvgl_ui.hpp
 */

#pragma once

#include <cstdint>

#include "sandbox/native_input.hpp"
#include "sandbox/native_lcd.hpp"

namespace sandbox {
struct ButtonCounters {
  uint32_t a = 0;
  uint32_t b = 0;
  uint32_t c = 0;
};

class NativeLvglUi {
 public:
  bool begin(NativeLcd *lcd);
  bool toggle_color_order();
  bool set_color_order(ColorOrder order);
  void update(const ButtonSnapshot &buttons, const ButtonCounters &counters, uint32_t uptime_ms);
  void tick(uint32_t elapsed_ms);
  void process();

 private:
  void rebuild_screen();
  static const char *color_order_name(ColorOrder order);
  static ColorOrder next_color_order_of(ColorOrder order);

  NativeLcd *lcd_ = nullptr;
  void *disp_ = nullptr;
  void *title_ = nullptr;
  void *subtitle_ = nullptr;
  void *footer_ = nullptr;
  ColorOrder color_order_ = ColorOrder::kBgr;
};
}  // namespace sandbox
