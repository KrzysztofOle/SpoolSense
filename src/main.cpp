#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <M5Unified.h>
#include <Wire.h>

namespace {
constexpr uint8_t kPn532SdaPin = 21;
constexpr uint8_t kPn532SclPin = 22;
constexpr uint8_t kPn532IrqPin = 255;    // Not used for this I2C diagnostic.
constexpr uint8_t kPn532ResetPin = 255;  // Not used for this I2C diagnostic.

Adafruit_PN532 *pn532 = nullptr;

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
}

void loop() {
  delay(1000);
}
