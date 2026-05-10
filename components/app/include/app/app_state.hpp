/**
 * Application state types shared by runtime and FSM.
 *
 * Features (EN):
 * - Defines application modes and subsystem status values.
 * - Keeps AppState and UiState in a dedicated shared header.
 *
 * Funkcje (PL):
 * - Definiuje tryby aplikacji i statusy podsystemow.
 * - Przenosi AppState oraz UiState do osobnego wspolnego naglowka.
 *
 * File: components/app/include/app/app_state.hpp
 */

#pragma once

#include <stdint.h>

#include "app/app_events.hpp"

namespace app {
struct HardwareAvailability {
  bool rfid_present = false;
  bool hx711_present = false;
  bool axp192_present = false;
};

enum class AppMode : uint8_t {
  kBoot,
  kIdle,
  kTagDetected,
  kMeasuring,
  kError,
  kCalibration,
};

enum class RfidStatus : uint8_t {
  kBooting,
  kReaderMissing,
  kWaitingForCard,
  kCardPresent,
  kCardRemoved,
};

enum class Hx711Status : uint8_t {
  kBooting,
  kDisabled,
  kNotFound,
  kReady,
};

struct AppState {
  HardwareAvailability hardware{};
  AppMode current_mode = AppMode::kBoot;
  RfidStatus rfid_status = RfidStatus::kBooting;
  Hx711Status hx711_status = Hx711Status::kBooting;
  bool hx711_enabled = true;
  bool rfid_has_uid = false;
  bool rfid_usage_available = false;
  bool rfid_page_read_failed = false;
  bool hx711_has_sample = false;
  bool hx711_zeroed = false;
  uint8_t rfid_uid[kMaxRfidUidLength] = {};
  uint8_t rfid_uid_length = 0;
  uint32_t rfid_firmware_version = 0;
  uint32_t rfid_usage_seconds = 0;
  uint8_t rfid_life_percent = 0;
  long hx711_raw_value = 0;
  int32_t hx711_weight_grams = 0;
  uint32_t rfid_last_change_ms = 0;
  uint32_t hx711_last_sample_ms = 0;
  uint32_t hx711_zeroed_ms = 0;
  ButtonKind last_button = ButtonKind::kNone;
  uint32_t last_button_ms = 0;
};

struct UiState {
  HardwareAvailability hardware{};
  AppMode current_mode = AppMode::kBoot;
  RfidStatus rfid_status = RfidStatus::kBooting;
  Hx711Status hx711_status = Hx711Status::kBooting;
  bool hx711_enabled = true;
  bool rfid_has_uid = false;
  bool rfid_usage_available = false;
  bool rfid_page_read_failed = false;
  bool hx711_has_sample = false;
  bool hx711_zeroed = false;
  uint8_t rfid_uid[kMaxRfidUidLength] = {};
  uint8_t rfid_uid_length = 0;
  uint32_t rfid_firmware_version = 0;
  uint32_t rfid_usage_seconds = 0;
  uint8_t rfid_life_percent = 0;
  long hx711_raw_value = 0;
  int32_t hx711_weight_grams = 0;
  uint32_t rfid_last_change_ms = 0;
  uint32_t hx711_last_sample_ms = 0;
  uint32_t hx711_zeroed_ms = 0;
  ButtonKind last_button = ButtonKind::kNone;
  uint32_t last_button_ms = 0;
};
}  // namespace app
