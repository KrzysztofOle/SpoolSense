/**
 * PN532 RFID helper that keeps the current tag handling behavior.
 *
 * Features (EN):
 * - Initializes PN532 over I2C.
 * - Polls tags and reports Sonicare usage data without changing behavior.
 *
 * Funkcje (PL):
 * - Inicjalizuje PN532 po I2C.
 * - Odpytuje tagi i raportuje dane uzycia Sonicare bez zmiany zachowania.
 *
 * File: components/pn532/src/pn532_reader.cpp
 */

#include "pn532/pn532_reader.hpp"

#include <Arduino.h>
#include <Wire.h>

#include "board/board.hpp"

namespace {
constexpr uint8_t kSonicareUsagePage = 0x24;
}  // namespace

Pn532Reader::Pn532Reader()
    : pn532_(board::kPn532IrqPin, board::kPn532ResetPin, &Wire),
      firmware_version_(0) {}

bool Pn532Reader::read_sonicare_usage_page(uint8_t *page_data) {
  board::I2cLock lock;
  return pn532_.ntag2xx_ReadPage(kSonicareUsagePage, page_data);
}

bool Pn532Reader::read_ntag_page(uint8_t page, uint8_t *page_data) {
  if (page_data == nullptr) {
    return false;
  }

  board::I2cLock lock;
  return pn532_.ntag2xx_ReadPage(page, page_data);
}

bool Pn532Reader::write_ntag_page(uint8_t page, const uint8_t *data) {
  if (data == nullptr) {
    return false;
  }

  uint8_t page_data[4] = {};
  for (size_t i = 0; i < sizeof(page_data); ++i) {
    page_data[i] = data[i];
  }

  board::I2cLock lock;
  return pn532_.ntag2xx_WritePage(page, page_data);
}

bool Pn532Reader::begin() {
  firmware_version_ = 0;

  board::I2cLock lock;
  board::begin_pn532_wire();
  board::log_i2c_scan();
  if (!pn532_.begin()) {
    return false;
  }

  firmware_version_ = pn532_.getFirmwareVersion();
  if (firmware_version_ == 0) {
    return false;
  }

  if (!pn532_.SAMConfig()) {
    firmware_version_ = 0;
    return false;
  }

  return true;
}

bool Pn532Reader::read_passive_target(uint8_t *uid, uint8_t *uid_length, uint16_t timeout_ms) {
  board::I2cLock lock;
  return pn532_.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uid_length, timeout_ms);
}
