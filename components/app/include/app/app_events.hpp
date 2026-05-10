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

enum class ButtonKind : uint8_t {
  kNone,
  kA,
  kB,
  kC,
};

enum class ButtonAction : uint8_t {
  kClick,
  kLongPress,
};

struct RfidEvent {
  enum class Kind : uint8_t {
    kReaderMissing,
    kReaderReady,
    kWaitingForCard,
    kCardRemoved,
    kCardPresent,
    kWriteResult,
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
  bool profile_available = false;
  char profile_material[16] = {};
  char profile_color[24] = {};
  uint16_t profile_reference_weight_g = 0;
  uint16_t profile_last_known_weight_g = 0;
  uint16_t profile_initial_filament_g = 0;
  uint16_t profile_spool_capacity_g = 0;
  uint16_t profile_diameter_x100 = 0;
  uint8_t profile_nozzle_temp_c = 0;
  uint8_t profile_bed_temp_c = 0;
  uint8_t profile_batch_id = 0;
  uint32_t profile_last_update_unix = 0;
  uint8_t profile_flags = 0;
  bool write_success = false;
  uint32_t timestamp_ms = 0;
};

struct WeightEvent {
  enum class Kind : uint8_t {
    kNotFound,
    kReady,
    kSample,
    kZeroed,
  };

  Kind kind = Kind::kNotFound;
  bool has_sample = false;
  long raw_value = 0;
  int32_t weight_grams = 0;
  uint32_t timestamp_ms = 0;
};

struct Hx711Command {
  enum class Kind : uint8_t {
    kZero,
  };

  Kind kind = Kind::kZero;
  uint32_t timestamp_ms = 0;
};

struct RfidCommand {
  enum class Kind : uint8_t {
    kWriteSampleData,
  };

  Kind kind = Kind::kWriteSampleData;
  uint8_t start_page = 0;
  uint8_t page_count = 0;
  uint8_t data[64] = {};
  uint32_t timestamp_ms = 0;
};

struct ButtonEvent {
  ButtonKind kind = ButtonKind::kNone;
  ButtonAction action = ButtonAction::kClick;
  uint32_t timestamp_ms = 0;
};
}  // namespace app
