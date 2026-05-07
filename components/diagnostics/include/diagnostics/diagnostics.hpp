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
 * File: components/diagnostics/include/diagnostics/diagnostics.hpp
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace diagnostics {
void log_line(const char *line);
void format_uid(const uint8_t *uid, uint8_t uid_length, char *buffer, size_t buffer_size);
void format_hex_bytes(const uint8_t *data, uint8_t data_length, char *buffer, size_t buffer_size);
const char *classify_tag_type(uint8_t uid_length);
uint16_t read_little_endian_u16(const uint8_t *data);
uint32_t sonicare_counts_to_seconds(uint16_t raw_count);
uint8_t estimate_life_percent(uint32_t usage_seconds);
void log_uid_and_type(const uint8_t *uid, uint8_t uid_length);
void log_uid_and_usage(const uint8_t *uid, uint8_t uid_length, const uint8_t *page_data,
                       uint32_t usage_seconds, uint8_t life_percent);
void log_raw_weight(long raw_value);
}  // namespace diagnostics
