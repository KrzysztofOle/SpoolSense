/**
 * Application orchestration layer.
 *
 * Features (EN):
 * - Owns the current firmware boot and loop orchestration.
 * - Keeps hardware-specific behavior inside the component modules.
 *
 * Funkcje (PL):
 * - Zarzadza obecnym bootem i petla aplikacji.
 * - Trzyma sprzetowe zachowanie wewnatrz modulow komponentow.
 *
 * File: components/app/src/app.cpp
 */

#include "app/app.hpp"

#include <Arduino.h>
#include <M5Unified.h>

#include "display/display.hpp"

namespace app {
void App::begin() {
  Serial.begin(115200);
  M5.begin();
  display::begin();

  pn532_reader_.begin();
  hx711_monitor_.begin();
}

void App::loop() {
  pn532_reader_.update();
  hx711_monitor_.update();
}
}  // namespace app
