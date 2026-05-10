/**
 * SpoolTagV1 record helpers for NFC payloads.
 *
 * Features (EN):
 * - Defines the stable packed SpoolTagV1 payload layout.
 * - Provides serialization, parsing, CRC16, and lightweight validation.
 * - Exposes NTAG page layout constants for read/write operations.
 *
 * Funkcje (PL):
 * - Definiuje stabilny, spakowany layout payloadu SpoolTagV1.
 * - Udostepnia serializacje, parsowanie, CRC16 i lekka walidacje.
 * - Udostepnia stale layoutu stron NTAG do operacji odczytu/zapisu.
 *
 * File: components/spool_tag/include/spool_tag/spool_tag.hpp
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace spool_tag {
constexpr uint8_t kSpoolTagV1StartPage = 8;
constexpr uint8_t kSpoolTagMagic0 = 'S';
constexpr uint8_t kSpoolTagMagic1 = 'S';
constexpr uint8_t kSpoolTagV1Version = 2;

#pragma pack(push, 1)
struct SpoolTagV1 {
  char magic[2];
  uint8_t version;
  char material[16];
  char color[24];
  uint16_t reference_weight_g;
  uint16_t last_known_weight_g;
  uint16_t initial_filament_g;
  uint16_t spool_capacity_g;
  uint16_t diameter_x100;
  uint8_t nozzle_temp_c;
  uint8_t bed_temp_c;
  uint8_t batch_id;
  uint32_t last_update_unix;
  uint8_t flags;
  uint16_t crc16;
};
#pragma pack(pop)

static_assert(sizeof(SpoolTagV1) == 63, "SpoolTagV1 size must be 63 bytes");

constexpr size_t kSpoolTagV1PayloadSize = sizeof(SpoolTagV1);
constexpr uint8_t kSpoolTagV1PageCount = static_cast<uint8_t>((kSpoolTagV1PayloadSize + 3U) / 4U);
constexpr uint8_t kSpoolTagV1EndPageExclusive =
    static_cast<uint8_t>(kSpoolTagV1StartPage + kSpoolTagV1PageCount);

struct ParsedSpoolTagV1 {
  bool valid = false;
  bool supported_version = false;
  bool crc_ok = false;
  SpoolTagV1 tag{};
};

uint16_t crc16_ccitt(const uint8_t *data, size_t size);

void init_tag(SpoolTagV1 &tag);

bool serialize(const SpoolTagV1 &tag, uint8_t *buffer, size_t size);

ParsedSpoolTagV1 parse(const uint8_t *buffer, size_t size);

void sanitize_ascii_field(char *dst, size_t dst_size, const char *src);

}  // namespace spool_tag
