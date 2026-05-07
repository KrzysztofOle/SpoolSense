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

namespace {
SemaphoreHandle_t g_i2c_mutex = nullptr;
}  // namespace

namespace board {
void begin_i2c_mutex() {
  if (g_i2c_mutex == nullptr) {
    g_i2c_mutex = xSemaphoreCreateMutex();
  }
}

void begin_pn532_wire() {
  Wire.begin(kPn532SdaPin, kPn532SclPin);
}

bool take_i2c(TickType_t timeout_ticks) {
  if (g_i2c_mutex == nullptr) {
    return true;
  }

  return xSemaphoreTake(g_i2c_mutex, timeout_ticks) == pdTRUE;
}

void give_i2c() {
  if (g_i2c_mutex != nullptr) {
    xSemaphoreGive(g_i2c_mutex);
  }
}

I2cLock::I2cLock(TickType_t timeout_ticks) : acquired_(take_i2c(timeout_ticks)) {}

I2cLock::~I2cLock() {
  if (acquired_) {
    give_i2c();
  }
}
}  // namespace board
