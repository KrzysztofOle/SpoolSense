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
 * File: components/display/src/display.cpp
 */

#include "display/display.hpp"

#include <Arduino.h>
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

#include "diagnostics/diagnostics.hpp"

namespace display {
void begin() {
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);
}

void show_text(const char *line1, const char *line2) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.println(line1);
  if (line2 != nullptr) {
    M5.Display.println(line2);
  }
}

void append_line(const char *line) {
  M5.Display.println(line);
}

void show_card_removed() {
  show_text("Card removed", "Waiting for card");
}

void show_uid_and_type(const uint8_t *uid, uint8_t uid_length) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char type_line[48] = {};
  snprintf(type_line, sizeof(type_line), "Type: %s", diagnostics::classify_tag_type(uid_length));

  show_text(uid_line, type_line);
}

void show_uid_and_usage(const uint8_t *uid, uint8_t uid_length, uint32_t usage_seconds,
                        uint8_t life_percent) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char usage_line[48] = {};
  snprintf(usage_line, sizeof(usage_line), "Usage: %lu s",
           static_cast<unsigned long>(usage_seconds));
  size_t used = strlen(usage_line);
  if (used + 1 < sizeof(usage_line)) {
    snprintf(usage_line + used, sizeof(usage_line) - used, " Life: %u%%",
             static_cast<unsigned>(life_percent));
  }

  show_text(uid_line, usage_line);
}

void show_page_read_failed() {
  append_line("Page 24: read failed");
}
}  // namespace display
