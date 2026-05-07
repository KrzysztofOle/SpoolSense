/**
 * Application event types shared by runtime and FSM.
 *
 * Features (EN):
 * - Defines RFID and HX711 event payloads.
 * - Keeps shared event data independent from runtime orchestration.
 *
 * Funkcje (PL):
 * - Definiuje struktury zdarzen RFID i HX711.
 * - Odlacza wspolne dane zdarzen od orkiestracji runtime.
 *
 * File: components/app/include/app/app_events.hpp
 */

#pragma once

#include <stdint.h>

namespace app {
constexpr uint8_t kMaxRfidUidLength = 10;

struct RfidEvent {
  enum class Kind : uint8_t {
    kReaderMissing,
    kReaderReady,
    kWaitingForCard,
    kCardRemoved,
    kCardPresent,
  };

  Kind kind = Kind::kReaderMissing;
  uint32_t firmware_version = 0;
  bool has_uid = false;
  bool usage_available = false;
  bool page_read_failed = false;
  uint8_t uid[kMaxRfidUidLength] = {};
  uint8_t uid_length = 0;
  uint8_t usage_page[4] = {};
  uint32_t usage_seconds = 0;
  uint8_t life_percent = 0;
  uint32_t timestamp_ms = 0;
};

struct WeightEvent {
  enum class Kind : uint8_t {
    kNotFound,
    kReady,
    kSample,
  };

  Kind kind = Kind::kNotFound;
  bool has_sample = false;
  long raw_value = 0;
  uint32_t timestamp_ms = 0;
};
}  // namespace app
