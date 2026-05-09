/**
 * PlatformIO sandbox app for M5Stack Core ESP32.
 *
 * Features (EN):
 * - Initializes M5Unified and draws a simple status screen.
 * - Counts button presses for BtnA, BtnB, and BtnC.
 * - Shows the application folder name on the display.
 *
 * Funkcje (PL):
 * - Inicjalizuje M5Unified i rysuje prosty ekran statusu.
 * - Zlicza nacisniecia przyciskow BtnA, BtnB i BtnC.
 * - Pokazuje na ekranie nazwe folderu aplikacji.
 *
 * File: sandbox/platformio/src/main.cpp
 */

#include <Arduino.h>
#include <M5Unified.h>

#include <cstdio>

namespace {
constexpr const char k_app_folder[] = "sandbox/platformio";

struct ButtonCounters {
  uint32_t a = 0;
  uint32_t b = 0;
  uint32_t c = 0;
};

ButtonCounters g_counters{};

void draw_screen(const ButtonCounters &counters) {
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE, BLACK);

  M5.Display.setTextSize(2);
  M5.Display.setCursor(16, 18);
  M5.Display.println(k_app_folder);

  M5.Display.setTextSize(3);
  M5.Display.setCursor(16, 64);
  M5.Display.printf("BtnA: %lu\n", static_cast<unsigned long>(counters.a));
  M5.Display.printf("BtnB: %lu\n", static_cast<unsigned long>(counters.b));
  M5.Display.printf("BtnC: %lu\n", static_cast<unsigned long>(counters.c));
}
}  // namespace

void setup() {
  Serial.begin(115200);

  auto cfg = M5.config();
  cfg.clear_display = true;
  cfg.fallback_board = m5::board_t::board_M5Stack;
  M5.begin(cfg);

  draw_screen(g_counters);

  Serial.println("Sandbox LCD ready");
}

void loop() {
  static bool initialized = false;
  static bool last_a = false;
  static bool last_b = false;
  static bool last_c = false;
  static bool input_armed = false;
  static uint8_t released_streak = 0;
  static uint32_t last_a_click_ms = 0;
  static uint32_t last_b_click_ms = 0;
  static uint32_t last_c_click_ms = 0;

  M5.update();

  const bool current_a = M5.BtnA.isPressed();
  const bool current_b = M5.BtnB.isPressed();
  const bool current_c = M5.BtnC.isPressed();
  const uint32_t now_ms = millis();
  bool redraw = !initialized;

  if (!input_armed) {
    if (!current_a && !current_b && !current_c) {
      if (released_streak < 5) {
        ++released_streak;
      }
      if (released_streak >= 5) {
        input_armed = true;
        last_a = current_a;
        last_b = current_b;
        last_c = current_c;
      }
    } else {
      released_streak = 0;
    }

    if (redraw) {
      draw_screen(g_counters);
      initialized = true;
    }

    delay(20);
    return;
  }

  if (current_a && !last_a && (now_ms - last_a_click_ms) > 250U) {
    ++g_counters.a;
    last_a_click_ms = now_ms;
    redraw = true;
    Serial.println("BtnA clicked");
  }
  if (current_b && !last_b && (now_ms - last_b_click_ms) > 250U) {
    ++g_counters.b;
    last_b_click_ms = now_ms;
    redraw = true;
    Serial.println("BtnB clicked");
  }
  if (current_c && !last_c && (now_ms - last_c_click_ms) > 250U) {
    ++g_counters.c;
    last_c_click_ms = now_ms;
    redraw = true;
    Serial.println("BtnC clicked");
  }

  if (redraw) {
    draw_screen(g_counters);
    initialized = true;
  }

  last_a = current_a;
  last_b = current_b;
  last_c = current_c;

  delay(20);
}
