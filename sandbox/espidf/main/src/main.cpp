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
  Serial.begin(115200);

  // Alternative startup path: rely on M5Unified autodetect defaults.
  M5.begin();
  M5.Display.setBrightness(255);

  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(16, 16);
  M5.Display.println("sandbox/espidf");
  M5.Display.setTextSize(3);
  M5.Display.setCursor(20, 32);
  M5.Display.println("Halo World");
  M5.Display.setCursor(20, 72);
  M5.Display.println("ESP-IDF");
  M5.Display.setCursor(20, 112);
  M5.Display.println("Alt");

  Serial.println("Sandbox ESP-IDF LCD ready");

  bool prev_a = false;
  bool prev_b = false;
  bool prev_c = false;

  for (;;) {
    M5.update();
    const bool btn_a = M5.BtnA.isPressed();
    const bool btn_b = M5.BtnB.isPressed();
    const bool btn_c = M5.BtnC.isPressed();

    if (btn_a != prev_a || btn_b != prev_b || btn_c != prev_c) {
      Serial.printf("Buttons A:%d B:%d C:%d\n",
                    static_cast<int>(btn_a),
                    static_cast<int>(btn_b),
                    static_cast<int>(btn_c));
      prev_a = btn_a;
      prev_b = btn_b;
      prev_c = btn_c;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
