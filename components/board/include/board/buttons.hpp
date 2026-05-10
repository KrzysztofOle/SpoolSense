/**
 * Board button helpers.
 *
 * Features (EN):
 * - Configures the board buttons as pulled-up GPIO inputs.
 * - Reads the current pressed state of the hardware buttons.
 *
 * Funkcje (PL):
 * - Konfiguruje przyciski plytki jako wejscia GPIO z pull-up.
 * - Odczytuje aktualny stan wcisniecia przyciskow sprzetowych.
 *
 * File: components/board/include/board/buttons.hpp
 */

#pragma once

#include <array>
#include <cstddef>
#include <stdint.h>

namespace board {
enum class ButtonKind : uint8_t {
  kNone,
  kA,
  kB,
  kC,
};

struct ButtonEvent {
  ButtonKind kind = ButtonKind::kNone;
  uint32_t timestamp_ms = 0;
};

struct ButtonEventBatch {
  std::array<ButtonEvent, 3> events{};
  size_t count = 0;
};

struct ButtonSnapshot {
  bool a = false;
  bool b = false;
  bool c = false;
};

class ButtonController {
 public:
  ButtonController() = default;

  void begin();
  ButtonEventBatch poll(uint32_t now_ms);

 private:
  static constexpr uint32_t kDebounceMs = 250U;

  ButtonSnapshot previous_snapshot_{};
  uint32_t last_action_a_ms_ = 0;
  uint32_t last_action_b_ms_ = 0;
  uint32_t last_action_c_ms_ = 0;
};
}  // namespace board
