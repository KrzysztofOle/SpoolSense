/**
 * Pure text renderer for the native sandbox app.
 *
 * Features (EN):
 * - Renders fixed-width text into 16-bit RGB scanlines.
 * - Draws a small animation bar for visual feedback.
 * - Keeps rendering code independent from the LCD transport layer.
 *
 * Funkcje (PL):
 * - Rysuje tekst o stalych szerokosciach do 16-bitowych linii RGB.
 * - Dodaje prosty pasek animacji jako sygnal wizualny.
 * - Odlacza renderowanie od warstwy transmisji LCD.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_renderer.hpp
 */

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace sandbox {
struct RenderLine {
  std::string_view text;
  int y = 0;
  uint16_t color = 0;
};

void render_scanline(const std::array<RenderLine, 4> &lines, int y, uint32_t animation_tick, uint16_t *line);
}  // namespace sandbox
