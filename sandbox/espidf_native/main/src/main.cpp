/**
 * Entry point for the native ESP-IDF sandbox application.
 *
 * Features (EN):
 * - Starts the modular native sandbox runtime.
 * - Keeps app_main minimal and platform-focused.
 *
 * Funkcje (PL):
 * - Uruchamia modularny native runtime sandboxa.
 * - Utrzymuje app_main jako minimalny punkt wejscia.
 *
 * File: sandbox/espidf_native/main/src/main.cpp
 */

#include "sandbox/native_app.hpp"

extern "C" void app_main(void) {
  sandbox::NativeApp app;
  app.run();
}
