/**
 * Board button helpers.
 *
 * Features (EN):
 * - Configures the board buttons as pulled-up GPIO inputs.
 * - Reads the current pressed state of the hardware buttons.
 * - Emits click and long-press button events.
 *
 * Funkcje (PL):
 * - Konfiguruje przyciski plytki jako wejscia GPIO z pull-up.
 * - Odczytuje aktualny stan wcisniecia przyciskow sprzetowych.
 * - Generuje zdarzenia click i long-press.
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

enum class ButtonAction : uint8_t {
  kClick,
  kLongPress,
};

struct ButtonEvent {
  ButtonKind kind = ButtonKind::kNone;
  ButtonAction action = ButtonAction::kClick;
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
  static constexpr uint32_t kLongPressThresholdMs = 700U;

  struct ButtonState {
    bool raw_pressed = false;
    bool stable_pressed = false;
    uint32_t last_raw_change_ms = 0;
    uint32_t pressed_since_ms = 0;
    bool long_press_sent = false;
  };

  ButtonState button_a_{};
  ButtonState button_b_{};
  ButtonState button_c_{};
};
}  // namespace board
