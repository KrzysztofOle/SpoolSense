#include <Arduino.h>

#if ENABLE_HW_TESTS
void runRfidTest() {
  // Legacy hardware test intentionally disabled while the PN532 logic now
  // lives in the dedicated pn532 component.
}
#endif
