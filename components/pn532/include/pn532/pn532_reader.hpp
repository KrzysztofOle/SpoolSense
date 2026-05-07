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
 * File: components/pn532/include/pn532/pn532_reader.hpp
 */

#pragma once

#include <Adafruit_PN532.h>
#include <stdint.h>

class Pn532Reader {
 public:
  Pn532Reader();

  void begin();
  void update();

 private:
  bool same_uid(const uint8_t *uid, uint8_t uid_length) const;
  void store_uid(const uint8_t *uid, uint8_t uid_length);
  bool read_sonicare_usage_page(uint8_t *page_data);

  Adafruit_PN532 pn532_;
  uint8_t last_uid_[10];
  uint8_t last_uid_length_;
  bool has_last_uid_;
  uint32_t last_card_seen_at_ms_;
  uint32_t next_card_poll_at_ms_;
};
