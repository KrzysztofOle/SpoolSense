/**
 * M5 display helpers for the current firmware UI.
 *
 * Features (EN):
 * - Initializes the M5 display.
 * - Renders status, RFID, and diagnostic messages.
 *
 * Funkcje (PL):
 * - Inicjalizuje wyswietlacz M5.
 * - Rysuje status, RFID oraz komunikaty diagnostyczne.
 *
 * File: components/display/include/display/display.hpp
 */

#pragma once

#include <stdint.h>

namespace display {
enum class UiScreen : uint8_t {
  kHome = 0,
  kDiagnostics = 1,
  kScale = 2,
};

struct HomeSnapshot {
  char material[12] = {};
  char color[16] = {};
  int32_t current_weight_g = 0;
  int32_t reference_full_weight_g = 0;
  int32_t used_weight_g = 0;
  uint8_t remaining_percent = 0;
};

struct DiagnosticsSnapshot {
  uint8_t hx711 = 0;
  uint8_t pn532 = 0;
  uint8_t display = 0;
  uint8_t i2c = 0;
};

struct ScaleSnapshot {
  int32_t current_weight_g = 0;
  int32_t reference_full_weight_g = 0;
  char status_message[32] = {};
};

struct UiSnapshot {
  UiScreen screen = UiScreen::kHome;
  HomeSnapshot home{};
  DiagnosticsSnapshot diagnostics{};
  ScaleSnapshot scale{};
};

void begin();
bool is_narrow();
void show_boot_test();
void show_text(const char *line1, const char *line2 = nullptr);
void show_lines(const char *line1, const char *line2 = nullptr, const char *line3 = nullptr,
                const char *line4 = nullptr);
void show_diagnostics(const char *line1, const char *line2, const char *line3, const char *line4);
void render_home(const HomeSnapshot &snapshot);
void render(const UiSnapshot &snapshot);
void append_line(const char *line);
void show_card_removed();
void show_uid_and_type(const uint8_t *uid, uint8_t uid_length);
void show_uid_and_usage(const uint8_t *uid, uint8_t uid_length, uint32_t usage_seconds,
                        uint8_t life_percent);
void show_page_read_failed();
}  // namespace display
