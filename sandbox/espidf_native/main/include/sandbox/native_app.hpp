/**
 * Native ESP-IDF sandbox application controller.
 *
 * Features (EN):
 * - Orchestrates LCD startup, button input, screens, and counters.
 * - Keeps the sandbox in a small, modular, ESP-IDF-only runtime.
 * - Provides the app_main entry path through a compact class.
 *
 * Funkcje (PL):
 * - Orkiestruje start LCD, wejscie z przyciskow, ekrany i liczniki.
 * - Utrzymuje sandbox w malym, modularnym runtime tylko dla ESP-IDF.
 * - Udostepnia wejscie app_main przez kompaktowa klase.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_app.hpp
 */

#pragma once

namespace sandbox {
class NativeApp {
 public:
  void run();
};
}  // namespace sandbox
