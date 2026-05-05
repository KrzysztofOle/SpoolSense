#include <Arduino.h>
#include <M5Unified.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  M5.begin();
  Serial.println("M5Stack Core initialized");
}

void loop() {
  Serial.println("Loop running");
  delay(1000);
}
