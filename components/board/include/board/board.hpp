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

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

namespace board {
constexpr uint8_t kPn532SdaPin = 21;
constexpr uint8_t kPn532SclPin = 22;
constexpr uint8_t kPn532IrqPin = 255;
constexpr uint8_t kPn532ResetPin = 255;
constexpr uint8_t kPn532I2cAddress = 0x24;
constexpr uint8_t kIp5306I2cAddress = 0x75;
constexpr uint8_t kAxp192I2cAddress = 0x34;
constexpr uint8_t kHx711DoutPin = 16;
constexpr uint8_t kHx711SckPin = 17;

void begin_pn532_wire();
void begin_i2c_mutex();
void log_i2c_scan();
bool take_i2c(TickType_t timeout_ticks = portMAX_DELAY);
void give_i2c();

class I2cLock {
 public:
  explicit I2cLock(TickType_t timeout_ticks = portMAX_DELAY);
  ~I2cLock();

  I2cLock(const I2cLock&) = delete;
  I2cLock& operator=(const I2cLock&) = delete;

  bool acquired() const {
    return acquired_;
  }

 private:
  bool acquired_;
};
}  // namespace board
