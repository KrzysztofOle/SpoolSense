#include <Arduino.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <Wire.h>

namespace {
constexpr uint8_t kPn532SdaPin = 21;
constexpr uint8_t kPn532SclPin = 22;
constexpr uint32_t kPn532PollIntervalMs = 700;
constexpr uint32_t kPn532InitRetryMs = 1500;

PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);

bool pn532Initialized = false;
bool pn532Ready = false;
uint32_t nextPollAtMs = 0;
uint32_t nextInitRetryAtMs = 0;

void printUid(const uint8_t *uid, uint8_t uidLength) {
  Serial.print("PN532 UID: ");
  for (uint8_t i = 0; i < uidLength; ++i) {
    if (i != 0) {
      Serial.print(' ');
    }
    if (uid[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(uid[i], HEX);
  }
  Serial.println();
}
}  // namespace

void runRfidTest() {
  if (!pn532Initialized) {
    Wire.begin(kPn532SdaPin, kPn532SclPin);
    nfc.begin();
    pn532Initialized = true;
    nextInitRetryAtMs = 0;
    Serial.println("PN532 I2C mode required, check DIP switch");
  }

  const uint32_t now = millis();

  if (!pn532Ready) {
    if (now < nextInitRetryAtMs) {
      delay(10);
      return;
    }

    const uint32_t firmwareVersion = nfc.getFirmwareVersion();
    if (firmwareVersion != 0 && nfc.SAMConfig()) {
      pn532Ready = true;
      nextPollAtMs = now;
      Serial.println("PN532 init OK");
    } else {
      Serial.println("PN532 not found");
      nextInitRetryAtMs = now + kPn532InitRetryMs;
      delay(100);
    }
    return;
  }

  if (now < nextPollAtMs) {
    delay(10);
    return;
  }

  uint8_t uid[7] = {};
  uint8_t uidLength = 0;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength,
                               100)) {
    printUid(uid, uidLength);
  }

  nextPollAtMs = now + kPn532PollIntervalMs;
  delay(20);
}
