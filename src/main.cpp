#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <M5Unified.h>
#include <Wire.h>
#include <cstring>

namespace {
constexpr uint8_t kPn532SdaPin = 21;
constexpr uint8_t kPn532SclPin = 22;
constexpr uint8_t kPn532IrqPin = 255;    // Not used for this I2C diagnostic.
constexpr uint8_t kPn532ResetPin = 255;  // Not used for this I2C diagnostic.
constexpr uint32_t kCardPollIntervalMs = 400;
constexpr uint32_t kCardGoneTimeoutMs = 1600;
constexpr uint16_t kReadTimeoutMs = 50;
constexpr uint8_t kMaxUidLength = 10;

Adafruit_PN532 *pn532 = nullptr;
uint8_t lastUid[kMaxUidLength] = {};
uint8_t lastUidLength = 0;
bool hasLastUid = false;
uint32_t lastCardSeenAtMs = 0;
uint32_t nextCardPollAtMs = 0;

void showText(const char *line1, const char *line2 = nullptr) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.println(line1);
  if (line2 != nullptr) {
    M5.Display.println(line2);
  }
}

void logLine(const char *line) {
  Serial.println(line);
  M5.Display.println(line);
}

void formatUid(const uint8_t *uid, uint8_t uidLength, char *buffer, size_t bufferSize) {
  size_t offset = 0;
  if (bufferSize == 0) {
    return;
  }

  buffer[0] = '\0';
  for (uint8_t i = 0; i < uidLength && offset + 3 < bufferSize; ++i) {
    const int written = snprintf(buffer + offset, bufferSize - offset,
                                 (i == 0) ? "%02X" : " %02X", uid[i]);
    if (written <= 0) {
      break;
    }
    offset += static_cast<size_t>(written);
  }
}

bool sameUid(const uint8_t *uid, uint8_t uidLength) {
  return hasLastUid && uidLength == lastUidLength &&
         memcmp(uid, lastUid, uidLength) == 0;
}

void storeUid(const uint8_t *uid, uint8_t uidLength) {
  memcpy(lastUid, uid, uidLength);
  lastUidLength = uidLength;
  hasLastUid = true;
}

void showUidOnDisplay(const uint8_t *uid, uint8_t uidLength) {
  char uidLine[48] = {};
  formatUid(uid, uidLength, uidLine, sizeof(uidLine));
  showText("RFID card detected", uidLine);
}

void printUidToSerial(const uint8_t *uid, uint8_t uidLength) {
  char uidLine[48] = {};
  formatUid(uid, uidLength, uidLine, sizeof(uidLine));
  Serial.print("UID: ");
  Serial.println(uidLine);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);

  showText("PN532 init test", "step 1");
  Serial.println("PN532 init test");
  Serial.println("step 1: Wire.begin");
  Wire.begin(kPn532SdaPin, kPn532SclPin);
  delay(1000);

  Serial.println("step 2: create PN532 object");
  pn532 = new Adafruit_PN532(kPn532IrqPin, kPn532ResetPin, &Wire);

  Serial.println("step 3: pn532.begin()");
  if (!pn532->begin()) {
    logLine("PN532 not found");
    return;
  }

  Serial.println("step 4: delay before firmware read");
  delay(500);

  Serial.println("step 5: getFirmwareVersion()");
  const uint32_t firmwareVersion = pn532->getFirmwareVersion();
  char versionLine[40] = {};
  snprintf(versionLine, sizeof(versionLine), "FW: 0x%08lX",
           static_cast<unsigned long>(firmwareVersion));
  Serial.println(versionLine);
  M5.Display.println(versionLine);

  if (firmwareVersion == 0) {
    logLine("PN532 not found");
    return;
  }

  Serial.println("step 6: SAMConfig()");
  if (!pn532->SAMConfig()) {
    logLine("PN532 not found");
    return;
  }

  showText("PN532 init OK", "Adafruit PN532");
  Serial.println("PN532 init OK");
  showText("PN532 init OK", "Waiting for card");
  nextCardPollAtMs = millis();
}

void loop() {
  if (pn532 == nullptr) {
    delay(1000);
    return;
  }

  const uint32_t now = millis();

  if (hasLastUid && (now - lastCardSeenAtMs >= kCardGoneTimeoutMs)) {
    hasLastUid = false;
    lastUidLength = 0;
    showText("Card removed", "Waiting for card");
    Serial.println("Card removed");
  }

  if (now < nextCardPollAtMs) {
    delay(10);
    return;
  }

  nextCardPollAtMs = now + kCardPollIntervalMs;

  uint8_t uid[kMaxUidLength] = {};
  uint8_t uidLength = 0;
  if (!pn532->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, kReadTimeoutMs)) {
    delay(10);
    return;
  }

  lastCardSeenAtMs = now;

  if (!sameUid(uid, uidLength)) {
    storeUid(uid, uidLength);
    printUidToSerial(uid, uidLength);
    showUidOnDisplay(uid, uidLength);
  }

  delay(10);
}
