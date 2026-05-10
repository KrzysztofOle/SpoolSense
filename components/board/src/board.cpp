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

#include <Arduino.h>
#include <cstdio>
#include <Wire.h>

#include "diagnostics/diagnostics.hpp"

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

bool probe_i2c_device(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

void log_i2c_scan() {
  diagnostics::log_line("I2C probe: begin");

  char line[64] = {};

  std::snprintf(line, sizeof(line), "I2C PN532 0x%02X: %s", kPn532I2cAddress,
                probe_i2c_device(kPn532I2cAddress) ? "present" : "missing");
  diagnostics::log_line(line);

  std::snprintf(line, sizeof(line), "I2C IP5306 0x%02X: %s", kIp5306I2cAddress,
                probe_i2c_device(kIp5306I2cAddress) ? "present" : "missing");
  diagnostics::log_line(line);

  std::snprintf(line, sizeof(line), "I2C AXP192 0x%02X: %s", kAxp192I2cAddress,
                probe_i2c_device(kAxp192I2cAddress) ? "present" : "missing");
  diagnostics::log_line(line);

  diagnostics::log_line("I2C probe: done");
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
