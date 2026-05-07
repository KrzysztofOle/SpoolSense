/**
 * PlatformIO bridge that pulls the component implementations into the Arduino build.
 *
 * Features (EN):
 * - Keeps the current PlatformIO Arduino build working while the real source
 *   of truth lives in the future ESP-IDF component tree.
 *
 * Funkcje (PL):
 * - Utrzymuje dzialajacy build Arduino w PlatformIO, podczas gdy docelowe
 *   zrodlo prawdy pozostaje w przyszlym drzewie komponentow ESP-IDF.
 *
 * File: src/components_bridge.cpp
 */

#include "../components/board/src/board.cpp"
#include "../components/diagnostics/src/diagnostics.cpp"
#include "../components/display/src/display.cpp"
#include "../components/hx711/src/hx711_monitor.cpp"
#include "../components/pn532/src/pn532_reader.cpp"
#include "../components/app/src/app.cpp"
