#include <Arduino.h>
#include <M5Unified.h>

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  cfg.clear_display = true;
  cfg.fallback_board = m5::board_t::board_M5Stack;
  M5.begin(cfg);

  M5.Display.setTextSize(3);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(20, 40);
  M5.Display.println("Halo World");
  M5.Display.setCursor(20, 80);
  M5.Display.println("Sandbox PIO");

  Serial.println("Sandbox LCD ready");
}

void loop() {
  delay(1000);
}
