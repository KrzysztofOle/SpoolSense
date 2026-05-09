/**
 * Pure text renderer for the native sandbox app.
 *
 * Features (EN):
 * - Converts fixed-width glyphs into RGB scanlines.
 * - Adds a compact animation bar for visual feedback.
 * - Remains independent from the LCD transport and button logic.
 *
 * Funkcje (PL):
 * - Konwertuje tekst o stalych szerokosciach do linii RGB.
 * - Dodaje kompaktowy pasek animacji jako sygnal wizualny.
 * - Jest niezalezny od transportu LCD oraz logiki przyciskow.
 *
 * File: sandbox/espidf_native/main/src/native_renderer.cpp
 */

#include "sandbox/native_renderer.hpp"

#include <array>

namespace sandbox {
namespace {
constexpr int k_lcd_width = 320;
constexpr int k_text_scale = 4;

constexpr uint16_t k_color_black = 0x0000;
constexpr uint16_t k_color_cyan = 0x07FF;

constexpr std::array<uint8_t, 5> glyph_for(char ch) {
  switch (ch) {
    case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19};
    case '1': return {0x00, 0x01, 0x1F, 0x00, 0x00};
    case '2': return {0x19, 0x15, 0x15, 0x15, 0x12};
    case '3': return {0x11, 0x15, 0x15, 0x15, 0x0A};
    case '4': return {0x07, 0x04, 0x04, 0x04, 0x1F};
    case '5': return {0x17, 0x15, 0x15, 0x15, 0x09};
    case '6': return {0x0E, 0x15, 0x15, 0x15, 0x08};
    case '7': return {0x10, 0x10, 0x10, 0x10, 0x1F};
    case '8': return {0x0A, 0x15, 0x15, 0x15, 0x0A};
    case '9': return {0x02, 0x15, 0x15, 0x15, 0x0E};
    case 'A': return {0x1E, 0x05, 0x05, 0x05, 0x1E};
    case 'B': return {0x1F, 0x15, 0x15, 0x15, 0x0A};
    case 'C': return {0x0E, 0x11, 0x11, 0x11, 0x11};
    case 'D': return {0x1F, 0x11, 0x11, 0x11, 0x0E};
    case 'E': return {0x1F, 0x15, 0x15, 0x15, 0x11};
    case 'F': return {0x1F, 0x05, 0x05, 0x05, 0x01};
    case 'G': return {0x0E, 0x11, 0x15, 0x15, 0x1D};
    case 'H': return {0x1F, 0x04, 0x04, 0x04, 0x1F};
    case 'I': return {0x11, 0x1F, 0x11, 0x00, 0x00};
    case 'L': return {0x1F, 0x10, 0x10, 0x10, 0x10};
    case 'M': return {0x1F, 0x02, 0x0C, 0x02, 0x1F};
    case 'N': return {0x1F, 0x02, 0x04, 0x08, 0x1F};
    case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x0E};
    case 'P': return {0x1F, 0x05, 0x05, 0x05, 0x02};
    case 'R': return {0x1F, 0x05, 0x0D, 0x15, 0x12};
    case 'S': return {0x12, 0x15, 0x15, 0x15, 0x09};
    case 'T': return {0x01, 0x01, 0x1F, 0x01, 0x01};
    case 'U': return {0x0F, 0x10, 0x10, 0x10, 0x0F};
    case 'V': return {0x07, 0x08, 0x10, 0x08, 0x07};
    case 'W': return {0x1F, 0x08, 0x04, 0x08, 0x1F};
    case 'X': return {0x1B, 0x04, 0x04, 0x04, 0x1B};
    case 'Y': return {0x03, 0x04, 0x18, 0x04, 0x03};
    case 'Z': return {0x19, 0x15, 0x13, 0x11, 0x11};
    case '-': return {0x04, 0x04, 0x04, 0x04, 0x04};
    case ':': return {0x00, 0x0A, 0x00, 0x0A, 0x00};
    case '/': return {0x10, 0x08, 0x04, 0x02, 0x01};
    case '.': return {0x00, 0x00, 0x00, 0x0C, 0x0C};
    case ' ': return {0x00, 0x00, 0x00, 0x00, 0x00};
    default: return {0x00, 0x00, 0x00, 0x00, 0x00};
  }
}

int text_width_px(std::string_view text, int scale) {
  if (text.empty()) {
    return 0;
  }

  return static_cast<int>(text.size()) * 6 * scale - scale;
}

void draw_text_scanline(std::string_view text, int x0, int y0, int scale, int y, uint16_t color,
                        uint16_t *line) {
  const int glyph_height = 7 * scale;
  const int rel_y = y - y0;
  if (rel_y < 0 || rel_y >= glyph_height) {
    return;
  }

  const int row = rel_y / scale;
  for (size_t index = 0; index < text.size(); ++index) {
    const auto glyph = glyph_for(text[index]);
    const int char_x = x0 + static_cast<int>(index) * 6 * scale;
    for (int col = 0; col < 5; ++col) {
      if (((glyph[col] >> row) & 0x01) == 0) {
        continue;
      }
      const int pixel_x = char_x + col * scale;
      for (int sx = 0; sx < scale; ++sx) {
        const int x = pixel_x + sx;
        if (x >= 0 && x < k_lcd_width) {
          line[x] = color;
        }
      }
    }
  }
}

void render_lines(const std::array<RenderLine, 4> &lines, int y, uint16_t *line) {
  for (const auto &entry : lines) {
    const int x = (k_lcd_width - text_width_px(entry.text, k_text_scale)) / 2;
    draw_text_scanline(entry.text, x, entry.y, k_text_scale, y, entry.color, line);
  }
}

void render_animation_bar(int y, uint32_t animation_tick, uint16_t *line) {
  constexpr int k_bar_top = 214;
  constexpr int k_bar_bottom = 224;
  constexpr int k_bar_width = 56;
  constexpr int k_bar_margin = 18;

  if (y < k_bar_top || y > k_bar_bottom) {
    return;
  }

  const int travel = k_lcd_width - (k_bar_width + 2 * k_bar_margin);
  const int offset = static_cast<int>(animation_tick % static_cast<uint32_t>(travel));
  const int bar_x = k_bar_margin + offset;

  for (int x = bar_x; x < bar_x + k_bar_width && x < k_lcd_width; ++x) {
    line[x] = k_color_cyan;
  }
}
}  // namespace

void render_scanline(const std::array<RenderLine, 4> &lines, int y, uint32_t animation_tick, uint16_t *line) {
  for (int x = 0; x < k_lcd_width; ++x) {
    line[x] = k_color_black;
  }

  render_lines(lines, y, line);
  render_animation_bar(y, animation_tick, line);
}
}  // namespace sandbox
