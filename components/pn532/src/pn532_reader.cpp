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
#include <cstdio>
#include <cstring>

#include "board/board.hpp"
#include "diagnostics/diagnostics.hpp"
#include "display/display.hpp"

namespace {
constexpr uint32_t kCardPollIntervalMs = 400;
constexpr uint32_t kCardGoneTimeoutMs = 1600;
constexpr uint16_t kReadTimeoutMs = 50;
constexpr uint8_t kMaxUidLength = 10;
constexpr uint8_t kSonicareUsagePage = 0x24;
}  // namespace

Pn532Reader::Pn532Reader()
    : pn532_(board::kPn532IrqPin, board::kPn532ResetPin, &Wire),
      last_uid_{},
      last_uid_length_(0),
      has_last_uid_(false),
      last_card_seen_at_ms_(0),
      next_card_poll_at_ms_(0) {}

bool Pn532Reader::same_uid(const uint8_t *uid, uint8_t uid_length) const {
  return has_last_uid_ && uid_length == last_uid_length_ &&
         memcmp(uid, last_uid_, uid_length) == 0;
}

void Pn532Reader::store_uid(const uint8_t *uid, uint8_t uid_length) {
  memcpy(last_uid_, uid, uid_length);
  last_uid_length_ = uid_length;
  has_last_uid_ = true;
}

bool Pn532Reader::read_sonicare_usage_page(uint8_t *page_data) {
  return pn532_.ntag2xx_ReadPage(kSonicareUsagePage, page_data);
}

void Pn532Reader::begin() {
  display::show_text("PN532 init test", "step 1");
  diagnostics::log_line("PN532 init test");
  diagnostics::log_line("step 1: Wire.begin");
  board::begin_pn532_wire();
  delay(1000);

  diagnostics::log_line("step 2: create PN532 object");
  diagnostics::log_line("step 3: pn532.begin()");
  if (!pn532_.begin()) {
    diagnostics::log_line("PN532 not found");
    display::show_text("PN532 not found");
    return;
  }

  diagnostics::log_line("step 4: delay before firmware read");
  delay(500);

  diagnostics::log_line("step 5: getFirmwareVersion()");
  const uint32_t firmware_version = pn532_.getFirmwareVersion();
  char version_line[40] = {};
  snprintf(version_line, sizeof(version_line), "FW: 0x%08lX",
           static_cast<unsigned long>(firmware_version));
  diagnostics::log_line(version_line);
  display::append_line(version_line);

  if (firmware_version == 0) {
    diagnostics::log_line("PN532 not found");
    display::show_text("PN532 not found");
    return;
  }

  diagnostics::log_line("step 6: SAMConfig()");
  if (!pn532_.SAMConfig()) {
    diagnostics::log_line("PN532 not found");
    display::show_text("PN532 not found");
    return;
  }

  display::show_text("PN532 init OK", "Adafruit PN532");
  diagnostics::log_line("PN532 init OK");
  display::show_text("PN532 init OK", "Waiting for card");
  next_card_poll_at_ms_ = millis();
}

void Pn532Reader::update() {
  const uint32_t now = millis();

  if (has_last_uid_ && (now - last_card_seen_at_ms_ >= kCardGoneTimeoutMs)) {
    has_last_uid_ = false;
    last_uid_length_ = 0;
    display::show_card_removed();
    diagnostics::log_line("Card removed");
  }

  if (now < next_card_poll_at_ms_) {
    delay(10);
    return;
  }

  next_card_poll_at_ms_ = now + kCardPollIntervalMs;

  uint8_t uid[kMaxUidLength] = {};
  uint8_t uid_length = 0;
  if (!pn532_.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_length, kReadTimeoutMs)) {
    delay(10);
    return;
  }

  last_card_seen_at_ms_ = now;

  if (!same_uid(uid, uid_length)) {
    store_uid(uid, uid_length);

    uint8_t usage_page[4] = {};
    if (read_sonicare_usage_page(usage_page)) {
      const uint16_t raw_count = diagnostics::read_little_endian_u16(usage_page);
      const uint32_t usage_seconds = diagnostics::sonicare_counts_to_seconds(raw_count);
      const uint8_t life_percent = diagnostics::estimate_life_percent(usage_seconds);

      diagnostics::log_uid_and_usage(uid, uid_length, usage_page, usage_seconds, life_percent);
      display::show_uid_and_usage(uid, uid_length, usage_seconds, life_percent);
    } else {
      diagnostics::log_uid_and_type(uid, uid_length);
      display::show_uid_and_type(uid, uid_length);
      diagnostics::log_line("Page 24: read failed");
      display::show_page_read_failed();
    }
  }

  delay(10);
}
