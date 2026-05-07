/**
 * Future ESP-IDF entry point scaffold.
 *
 * Features (EN):
 * - Hosts the native app_main entry point for the migration path.
 *
 * Funkcje (PL):
 * - Zawiera natywny punkt wejsciowy app_main dla sciezki migracji.
 *
 * File: main/main.cpp
 */

#include <Arduino.h>

#include "app/app.hpp"

extern "C" void app_main(void) {
  initArduino();

  static app::App app;
  app.begin();

  for (;;) {
    app.loop();
    delay(1);
  }
}
