/**
 * Board pin assignments and low-level board helpers.
 *
 * Features (EN):
 * - Centralizes PN532 and HX711 pin assignments.
 * - Provides the board-level I2C bootstrap for the PN532 bus.
 *
 * Funkcje (PL):
 * - Centralizuje przypisania pinow PN532 i HX711.
 * - Udostepnia boardowy bootstrap I2C dla magistrali PN532.
 *
 * File: components/board/include/board/board.hpp
 */

#pragma once

#include <stdint.h>

namespace board {
constexpr uint8_t kPn532SdaPin = 21;
constexpr uint8_t kPn532SclPin = 22;
constexpr uint8_t kPn532IrqPin = 255;
constexpr uint8_t kPn532ResetPin = 255;
constexpr uint8_t kHx711DoutPin = 16;
constexpr uint8_t kHx711SckPin = 4;

void begin_pn532_wire();
}  // namespace board
