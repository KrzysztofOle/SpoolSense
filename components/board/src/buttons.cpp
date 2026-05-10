/**
 * Board button GPIO helpers.
 *
 * Features (EN):
 * - Configures button pins for active-low input reading.
 * - Exposes a small helper for rising pressed edges.
 *
 * Funkcje (PL):
 * - Konfiguruje piny przyciskow do odczytu aktywnego stanu niskiego.
 * - Udostepnia maly helper dla zbocza wcisniecia.
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

bool is_pressed_edge(bool current, bool previous) {
  return current && !previous;
}
}  // namespace

void ButtonController::begin() {
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
  const struct {
    ButtonKind kind;
    bool pressed;
    bool previous;
    uint32_t *last_action_ms;
  } button_events[] = {
      {ButtonKind::kA, buttons.a, previous_snapshot_.a, &last_action_a_ms_},
      {ButtonKind::kB, buttons.b, previous_snapshot_.b, &last_action_b_ms_},
      {ButtonKind::kC, buttons.c, previous_snapshot_.c, &last_action_c_ms_},
  };

  ButtonEventBatch batch{};
  for (const auto &button_event : button_events) {
    if (!is_pressed_edge(button_event.pressed, button_event.previous)) {
      continue;
    }

    if ((now_ms - *button_event.last_action_ms) < kDebounceMs) {
      continue;
    }

    *button_event.last_action_ms = now_ms;
    batch.events[batch.count].kind = button_event.kind;
    batch.events[batch.count].timestamp_ms = now_ms;
    ++batch.count;
  }

  previous_snapshot_ = buttons;
  return batch;
}
}  // namespace board
