/**
 * Screen composition helpers for the native sandbox app.
 *
 * Features (EN):
 * - Defines the simple screen modes used by the sandbox UI.
 * - Builds text lines for the welcome, status, buttons, and menu screens.
 * - Keeps visual structure separate from application control flow.
 *
 * Funkcje (PL):
 * - Definiuje proste tryby ekranow uzywane przez UI sandboxa.
 * - Buduje linie tekstu dla ekranow welcome, status, buttons i menu.
 * - Odlacza strukture wizualna od logiki sterujacej aplikacji.
 *
 * File: sandbox/espidf_native/main/include/sandbox/native_screens.hpp
 */

#pragma once

#include <array>
#include <cstdint>

#include "sandbox/native_input.hpp"
#include "sandbox/native_renderer.hpp"

namespace sandbox {
enum class ScreenMode : uint8_t {
  kWelcome,
  kStatus,
  kButtons,
  kMenu,
};

struct ButtonCounters {
  uint32_t a = 0;
  uint32_t b = 0;
  uint32_t c = 0;
};

std::array<RenderLine, 4> build_lines(ScreenMode screen, uint32_t uptime_ms, uint32_t frame_count,
                                      const ButtonSnapshot &buttons, const ButtonCounters &counters);
const char *screen_name(ScreenMode screen);
}  // namespace sandbox
