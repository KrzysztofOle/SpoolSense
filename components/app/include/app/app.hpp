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
 * File: components/app/include/app/app.hpp
 */

#pragma once

#include "hx711/hx711_monitor.hpp"
#include "pn532/pn532_reader.hpp"

namespace app {
class App {
 public:
  App() = default;

  void begin();
  void loop();

 private:
  Pn532Reader pn532_reader_;
  Hx711Monitor hx711_monitor_;
};
}  // namespace app
