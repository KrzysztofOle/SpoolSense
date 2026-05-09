/**
 * Standalone ESP-IDF LCD hello world for M5Stack Core.
 *
 * Features (EN):
 * - Starts Arduino compatibility inside ESP-IDF.
 * - Shows a minimal splash screen on the LCD.
 *
 * Funkcje (PL):
 * - Uruchamia zgodnosc Arduino w ESP-IDF.
 * - Wyswietla prosty ekran startowy na LCD.
 *
 * File: sandbox/espidf/main/src/main.cpp
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <M5Unified.h>

extern "C" void app_main(void) {
  initArduino();

  auto cfg = M5.config();
  cfg.clear_display = true;
  cfg.fallback_board = m5::board_t::board_M5Stack;
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  M5.Display.setTextSize(3);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(20, 32);
  M5.Display.println("Halo World");
  M5.Display.setCursor(20, 72);
  M5.Display.println("ESP-IDF");
  M5.Display.setCursor(20, 112);
  M5.Display.println("Sandbox");

  Serial.println("Sandbox ESP-IDF LCD ready");

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
