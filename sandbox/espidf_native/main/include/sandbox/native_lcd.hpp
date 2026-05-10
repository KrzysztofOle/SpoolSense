/**
 * Native ESP-IDF LCD helper for the sandbox app.
 *
 * Features (EN):
 * - Initializes the classic M5Stack Core LCD over native ESP-IDF APIs.
 * - Sends scanlines directly through esp_lcd.
 * - Exposes a small hardware-focused interface for the sandbox runtime.
 *
 * Funkcje (PL):
 * - Inicjalizuje klasyczny LCD M5Stack Core przez natywne API ESP-IDF.
 * - Wysyla linie obrazu bezposrednio przez esp_lcd.
 * - Udostepnia maly interfejs sprzetowy dla runtime sandboxa.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_lcd.hpp
 */

#pragma once

#include <cstdint>

#include "esp_err.h"
#include "esp_lcd_panel_io.h"

namespace sandbox {
enum class ColorOrder : uint8_t {
  kRgb = 0,
  kBgr = 1,
};

class NativeLcd {
 public:
  static constexpr int kWidth = 320;
  static constexpr int kHeight = 240;

  esp_err_t begin();
  esp_err_t set_color_order(ColorOrder order);
  esp_err_t send_line(int y, const uint16_t *line);
  esp_err_t send_area(int x1, int y1, int x2, int y2, const uint16_t *pixels);
  bool wait_tx_done();

 private:
  ColorOrder color_order_ = ColorOrder::kRgb;
  esp_lcd_panel_io_handle_t io_handle_ = nullptr;
};
}  // namespace sandbox
