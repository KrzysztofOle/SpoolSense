#include <Arduino.h>

#if ENABLE_HW_TESTS
void runRfidTest() {
  // Legacy hardware test intentionally disabled while the PN532 init
  // diagnostic runs from src/main.cpp with Adafruit PN532.
}
#endif
