#include <Arduino.h>
#include <HX711.h>

#if ENABLE_HW_TESTS

namespace {
constexpr uint8_t kHx711DoutPin = 16;
constexpr uint8_t kHx711SckPin = 4;
constexpr uint32_t kHx711SampleIntervalMs = 700;
constexpr uint32_t kHx711InitRetryMs = 1500;

HX711 scale;
bool scaleInitialized = false;
bool scaleReady = false;
uint32_t nextSampleAtMs = 0;
uint32_t nextInitRetryAtMs = 0;

void printRawWeight(long rawValue) {
  Serial.print("HX711 raw: ");
  Serial.println(rawValue);
}
}  // namespace

void runHx711Test() {
  if (!scaleInitialized) {
    scale.begin(kHx711DoutPin, kHx711SckPin);
    scaleInitialized = true;
    nextInitRetryAtMs = 0;
  }

  const uint32_t now = millis();

  if (!scaleReady) {
    if (now < nextInitRetryAtMs) {
      delay(10);
      return;
    }

    if (scale.is_ready()) {
      scaleReady = true;
      nextSampleAtMs = now;
      Serial.println("HX711 init OK");
    } else {
      Serial.println("HX711 not found");
      nextInitRetryAtMs = now + kHx711InitRetryMs;
      delay(100);
    }
    return;
  }

  if (now < nextSampleAtMs) {
    delay(10);
    return;
  }

  if (scale.is_ready()) {
    printRawWeight(scale.read());
    nextSampleAtMs = now + kHx711SampleIntervalMs;
  } else {
    scaleReady = false;
    nextInitRetryAtMs = now + 250;
    Serial.println("HX711 not found");
  }

  delay(20);
}

#endif
