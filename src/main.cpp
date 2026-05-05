#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);
  Wire.begin(21, 22);  // SDA, SCL dla M5Stack

  M5.Display.setCursor(0, 0);
  M5.Display.println("I2C Scanner start");
  Serial.println("I2C Scanner start");
}

void loop() {
  static int cursorY = 24;
  static bool headerNeedsPrint = true;
  const int lineHeight = 20;

  if (headerNeedsPrint) {
    M5.Display.setCursor(0, 0);
    M5.Display.println("I2C scan");
    headerNeedsPrint = false;
  }

  for (uint8_t address = 0x01; address <= 0x7F; ++address) {
    Wire.beginTransmission(address);
    uint8_t result = Wire.endTransmission();

    if (result == 0) {
      if (cursorY + lineHeight > M5.Display.height()) {
        M5.Display.clear();
        cursorY = 24;
        headerNeedsPrint = true;
        M5.Display.setCursor(0, 0);
        M5.Display.println("I2C scan");
        headerNeedsPrint = false;
      }

      M5.Display.setCursor(0, cursorY);
      M5.Display.printf("Found: 0x%02X\n", address);
      cursorY += lineHeight;
      Serial.printf("Found device at 0x%02X\n", address);
    }
  }

  M5.Display.println("Scan done");
  Serial.println("Scan done");
  delay(3000);
}
