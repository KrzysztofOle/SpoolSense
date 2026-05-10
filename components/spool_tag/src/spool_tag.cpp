/**
 * SpoolTagV1 payload implementation for NFC records.
 *
 * Features (EN):
 * - Implements CRC16-CCITT checksum and packed binary serialization.
 * - Validates magic, version, and checksum before exposing parsed values.
 * - Sanitizes fixed ASCII text fields used in NFC payloads.
 *
 * Funkcje (PL):
 * - Implementuje CRC16-CCITT oraz spakowana serializacje binarna.
 * - Waliduje magic, version i checksum przed udostepnieniem danych.
 * - Sanityzuje stale pola ASCII stosowane w payloadzie NFC.
 *
 * File: components/spool_tag/src/spool_tag.cpp
 */

#include "spool_tag/spool_tag.hpp"

#include <ctype.h>
#include <string.h>

namespace spool_tag {
namespace {
constexpr uint16_t kCrcInit = 0xFFFF;
constexpr uint16_t kCrcPoly = 0x1021;
}

uint16_t crc16_ccitt(const uint8_t *data, size_t size) {
  if (data == nullptr) {
    return 0;
  }

  uint16_t crc = kCrcInit;
  for (size_t i = 0; i < size; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      if ((crc & 0x8000U) != 0U) {
        crc = static_cast<uint16_t>((crc << 1) ^ kCrcPoly);
      } else {
        crc = static_cast<uint16_t>(crc << 1);
      }
    }
  }
  return crc;
}

void sanitize_ascii_field(char *dst, size_t dst_size, const char *src) {
  if (dst == nullptr || dst_size == 0U) {
    return;
  }

  memset(dst, 0, dst_size);
  if (src == nullptr) {
    return;
  }

  size_t out = 0;
  while (src[out] != '\0' && out < (dst_size - 1U)) {
    const unsigned char ch = static_cast<unsigned char>(src[out]);
    dst[out] = (isascii(ch) != 0 && isprint(ch) != 0) ? static_cast<char>(ch) : '_';
    ++out;
  }
}

void init_tag(SpoolTagV1 &tag) {
  memset(&tag, 0, sizeof(tag));
  tag.magic[0] = static_cast<char>(kSpoolTagMagic0);
  tag.magic[1] = static_cast<char>(kSpoolTagMagic1);
  tag.version = kSpoolTagV1Version;
}

bool serialize(const SpoolTagV1 &tag, uint8_t *buffer, size_t size) {
  if (buffer == nullptr || size < kSpoolTagV1PayloadSize) {
    return false;
  }

  SpoolTagV1 encoded = tag;
  encoded.magic[0] = static_cast<char>(kSpoolTagMagic0);
  encoded.magic[1] = static_cast<char>(kSpoolTagMagic1);
  encoded.version = kSpoolTagV1Version;
  encoded.crc16 = 0;
  encoded.crc16 = crc16_ccitt(reinterpret_cast<const uint8_t *>(&encoded),
                              offsetof(SpoolTagV1, crc16));

  memcpy(buffer, &encoded, sizeof(encoded));
  return true;
}

ParsedSpoolTagV1 parse(const uint8_t *buffer, size_t size) {
  ParsedSpoolTagV1 result{};
  if (buffer == nullptr || size < kSpoolTagV1PayloadSize) {
    return result;
  }

  memcpy(&result.tag, buffer, sizeof(result.tag));

  if (static_cast<uint8_t>(result.tag.magic[0]) != kSpoolTagMagic0 ||
      static_cast<uint8_t>(result.tag.magic[1]) != kSpoolTagMagic1) {
    return result;
  }

  result.supported_version = (result.tag.version == kSpoolTagV1Version);
  if (!result.supported_version) {
    return result;
  }

  const uint16_t expected_crc = crc16_ccitt(buffer, offsetof(SpoolTagV1, crc16));
  result.crc_ok = (expected_crc == result.tag.crc16);
  result.valid = result.supported_version && result.crc_ok;
  return result;
}

}  // namespace spool_tag
