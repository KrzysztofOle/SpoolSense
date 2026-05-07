/**
 * Serial diagnostics and formatting helpers.
 *
 * Features (EN):
 * - Formats RFID identifiers and usage values.
 * - Emits serial debug logs for PN532 and HX711 flows.
 *
 * Funkcje (PL):
 * - Formatuje identyfikatory RFID i wartosci zuzycia.
 * - Wypisuje logi debugowe dla przeplywow PN532 i HX711.
 *
 * File: components/diagnostics/src/diagnostics.cpp
 */

#include "diagnostics/diagnostics.hpp"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

namespace {
constexpr uint32_t kSonicareSecondsPerCountX100 = 2055;
constexpr uint32_t kSonicareEstimatedLifeSeconds = 22000;

void format_bytes(const uint8_t *data, uint8_t data_length, char *buffer, size_t buffer_size) {
  size_t offset = 0;
  if (buffer_size == 0) {
    return;
  }

  buffer[0] = '\0';
  for (uint8_t i = 0; i < data_length && offset + 3 < buffer_size; ++i) {
    const int written = snprintf(buffer + offset, buffer_size - offset,
                                 (i == 0) ? "%02X" : " %02X", data[i]);
    if (written <= 0) {
      break;
    }
    offset += static_cast<size_t>(written);
  }
}
}  // namespace

namespace diagnostics {
void log_line(const char *line) {
  Serial.println(line);
}

void format_uid(const uint8_t *uid, uint8_t uid_length, char *buffer, size_t buffer_size) {
  format_bytes(uid, uid_length, buffer, buffer_size);
}

void format_hex_bytes(const uint8_t *data, uint8_t data_length, char *buffer, size_t buffer_size) {
  format_bytes(data, data_length, buffer, buffer_size);
}

const char *classify_tag_type(uint8_t uid_length) {
  switch (uid_length) {
    case 4:
      return "Mifare Classic / ISO14443A";
    case 7:
      return "NTAG / Ultralight";
    default:
      return "Unknown NFC tag";
  }
}

uint16_t read_little_endian_u16(const uint8_t *data) {
  return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t sonicare_counts_to_seconds(uint16_t raw_count) {
  return (static_cast<uint32_t>(raw_count) * kSonicareSecondsPerCountX100 + 50U) / 100U;
}

uint8_t estimate_life_percent(uint32_t usage_seconds) {
  const uint32_t percent =
      (usage_seconds * 100U + (kSonicareEstimatedLifeSeconds / 2U)) / kSonicareEstimatedLifeSeconds;
  return static_cast<uint8_t>(percent > 100U ? 100U : percent);
}

void log_uid_and_type(const uint8_t *uid, uint8_t uid_length) {
  char uid_line[48] = {};
  format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  Serial.print("UID: ");
  Serial.println(uid_line);
  Serial.print("Type: ");
  Serial.println(classify_tag_type(uid_length));
}

void log_uid_and_usage(const uint8_t *uid, uint8_t uid_length, const uint8_t *page_data,
                       uint32_t usage_seconds, uint8_t life_percent) {
  char uid_line[48] = {};
  format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  Serial.print("UID: ");
  Serial.println(uid_line);

  char page_line[32] = {};
  format_hex_bytes(page_data, 4, page_line, sizeof(page_line));
  Serial.print("Page 24: ");
  Serial.println(page_line);

  Serial.print("Usage: ");
  Serial.print(static_cast<unsigned long>(usage_seconds));
  Serial.println(" s");

  Serial.print("Life: ");
  Serial.print(static_cast<unsigned>(life_percent));
  Serial.println('%');

  Serial.print("RFID: UID=");
  Serial.print(uid_line);
  Serial.print(" Usage=");
  Serial.print(static_cast<unsigned long>(usage_seconds));
  Serial.print("s Life=");
  Serial.print(static_cast<unsigned>(life_percent));
  Serial.println('%');

  Serial.print("Type: ");
  Serial.println(classify_tag_type(uid_length));
}

void log_raw_weight(long raw_value) {
  Serial.print("HX711 raw: ");
  Serial.println(raw_value);
}
}  // namespace diagnostics
