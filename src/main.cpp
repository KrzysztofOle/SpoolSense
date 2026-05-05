#include <Arduino.h>
#include <M5Unified.h>

#define ENABLE_HW_TESTS 0

#if ENABLE_HW_TESTS
void runHx711Test();
void runRfidTest();
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  M5.begin();
  Serial.println("M5Stack Core initialized");

#if ENABLE_HW_TESTS
  Serial.println("Hardware test mode enabled");
#else
  Serial.println("Hardware test mode disabled");
#endif
}

void loop() {
#if ENABLE_HW_TESTS
  runHx711Test();
  runRfidTest();
  delay(500);
#else
  Serial.println("Loop running");
  delay(1000);
#endif
}
