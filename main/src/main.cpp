/**
 * ESP-IDF entry point for the current firmware flow.
 *
 * Features (EN):
 * - Hosts the native app_main entry point.
 *
 * Funkcje (PL):
 * - Zawiera natywny punkt wejsciowy app_main.
 *
 * File: main/src/main.cpp
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "app/app.hpp"

extern "C" void app_main(void) {
  initArduino();

  static app::App app;
  app.begin();

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
