/**
 * Board-level hardware bootstrap helpers.
 *
 * Features (EN):
 * - Starts the PN532 I2C bus using the board pin layout.
 *
 * Funkcje (PL):
 * - Uruchamia magistrale I2C PN532 zgodnie z ukladem pinow plytki.
 *
 * File: components/board/src/board.cpp
 */

#include "board/board.hpp"

#include <Wire.h>

namespace board {
void begin_pn532_wire() {
  Wire.begin(kPn532SdaPin, kPn532SclPin);
}
}  // namespace board
