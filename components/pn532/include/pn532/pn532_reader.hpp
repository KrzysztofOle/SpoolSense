/**
 * PN532 RFID access helper for the task-based runtime.
 *
 * Features (EN):
 * - Initializes PN532 over I2C.
 * - Exposes low-level tag and page reads for the RFID task.
 *
 * Funkcje (PL):
 * - Inicjalizuje PN532 po I2C.
 * - Udostepnia niskopoziomowe odczyty tagow i stron dla taska RFID.
 *
 * File: components/pn532/include/pn532/pn532_reader.hpp
 */

#pragma once

#include <Adafruit_PN532.h>
#include <stdint.h>

class Pn532Reader {
 public:
  Pn532Reader();

  bool begin();
  uint32_t firmware_version() const {
    return firmware_version_;
  }
  bool read_passive_target(uint8_t *uid, uint8_t *uid_length, uint16_t timeout_ms);
  bool read_sonicare_usage_page(uint8_t *page_data);
  bool read_ntag_page(uint8_t page, uint8_t *page_data);
  bool write_ntag_page(uint8_t page, const uint8_t *data);

 private:
  Adafruit_PN532 pn532_;
  uint32_t firmware_version_;
};
