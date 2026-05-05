#include <Arduino.h>
#include <M5Unified.h>

#ifndef ENABLE_HW_TESTS
#define ENABLE_HW_TESTS 0
#endif

#if ENABLE_HW_TESTS
void runHx711Test();
void runRfidTest();
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  M5.begin();
  M5.Display.clear();
  M5.Display.setTextSize(3);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Hello World");

  Serial.println("Hello World");
}

void loop() {
#if ENABLE_HW_TESTS
  runHx711Test();
  runRfidTest();
#else
  static uint32_t counter = 0;

  Serial.printf("Counter: %lu\n", static_cast<unsigned long>(counter));

  M5.Display.fillRect(10, 50, 220, 40, BLACK);
  M5.Display.setCursor(10, 50);
  M5.Display.printf("Count: %lu", static_cast<unsigned long>(counter));

  counter++;
  delay(1000);
#endif
}
