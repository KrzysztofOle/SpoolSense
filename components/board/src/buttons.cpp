/**
 * Board button GPIO helpers.
 *
 * Features (EN):
 * - Configures button pins for active-low input reading.
 * - Emits debounced click and long-press button events.
 *
 * Funkcje (PL):
 * - Konfiguruje piny przyciskow do odczytu aktywnego stanu niskiego.
 * - Generuje zdarzenia click i long-press z debouncingiem.
 *
 * File: components/board/src/buttons.cpp
 */

#include "board/buttons.hpp"

#include <driver/gpio.h>

#include "board/board.hpp"

namespace board {
namespace {
ButtonSnapshot read_buttons() {
  ButtonSnapshot snapshot{};
  snapshot.a = gpio_get_level(static_cast<gpio_num_t>(kBtnAPin)) == 0;
  snapshot.b = gpio_get_level(static_cast<gpio_num_t>(kBtnBPin)) == 0;
  snapshot.c = gpio_get_level(static_cast<gpio_num_t>(kBtnCPin)) == 0;
  return snapshot;
}
}
}  // namespace

void ButtonController::begin() {
  button_a_ = {};
  button_b_ = {};
  button_c_ = {};

  gpio_config_t io_config = {};
  io_config.intr_type = GPIO_INTR_DISABLE;
  io_config.mode = GPIO_MODE_INPUT;
  io_config.pin_bit_mask = (1ULL << kBtnAPin) | (1ULL << kBtnBPin) | (1ULL << kBtnCPin);
  io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_config.pull_up_en = GPIO_PULLUP_ENABLE;
  (void)gpio_config(&io_config);
}

ButtonEventBatch ButtonController::poll(uint32_t now_ms) {
  const ButtonSnapshot buttons = read_buttons();
  ButtonEventBatch batch{};
  const auto process_button = [now_ms, &batch](ButtonKind kind, bool raw_pressed, ButtonState &state) {
    if (raw_pressed != state.raw_pressed) {
      state.raw_pressed = raw_pressed;
      state.last_raw_change_ms = now_ms;
      if (raw_pressed) {
        state.pressed_since_ms = now_ms;
        state.long_press_sent = false;
      }
    }

    if (state.stable_pressed != state.raw_pressed &&
        (now_ms - state.last_raw_change_ms) >= kDebounceMs) {
      state.stable_pressed = state.raw_pressed;
      if (!state.stable_pressed && !state.long_press_sent) {
        if (batch.count < batch.events.size()) {
          batch.events[batch.count].kind = kind;
          batch.events[batch.count].action = ButtonAction::kClick;
          batch.events[batch.count].timestamp_ms = now_ms;
          ++batch.count;
        }
      }
    }

    if (state.stable_pressed && !state.long_press_sent &&
        (now_ms - state.pressed_since_ms) >= kLongPressThresholdMs) {
      if (batch.count < batch.events.size()) {
        batch.events[batch.count].kind = kind;
        batch.events[batch.count].action = ButtonAction::kLongPress;
        batch.events[batch.count].timestamp_ms = now_ms;
        ++batch.count;
        state.long_press_sent = true;
      }
    }
  };

  process_button(ButtonKind::kA, buttons.a, button_a_);
  process_button(ButtonKind::kB, buttons.b, button_b_);
  process_button(ButtonKind::kC, buttons.c, button_c_);
  return batch;
}
}  // namespace board
