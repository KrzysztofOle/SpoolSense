/**
 * Arduino entry point wrapper for the current firmware flow.
 *
 * Features (EN):
 * - Keeps setup() and loop() as thin wrappers.
 * - Delegates all behavior to the app orchestration component.
 *
 * Funkcje (PL):
 * - Utrzymuje setup() i loop() jako cienkie wrappery.
 * - Przekazuje caly przebieg do komponentu orkiestracji aplikacji.
 *
 * File: src/main.cpp
 */

#include <Arduino.h>

#include "app/app.hpp"

namespace {
app::App g_app;
}  // namespace

void setup() {
  g_app.begin();
}

void loop() {
  g_app.loop();
}
