/**
 * M5 display helpers for the current firmware UI.
 *
 * Features (EN):
 * - Initializes the M5 display.
 * - Renders status, RFID, and diagnostic messages.
 *
 * Funkcje (PL):
 * - Inicjalizuje wyswietlacz M5.
 * - Rysuje status, RFID oraz komunikaty diagnostyczne.
 *
 * File: components/display/src/display.cpp
 */

#include "display/display.hpp"

#include <Arduino.h>
#include <M5GFX.h>
#include <cstdio>
#include <cstring>

#include "lgfx/v1/panel/Panel_ILI9342.hpp"
#include "lgfx/v1/platforms/esp32/Bus_SPI.hpp"
#include "lgfx/v1/platforms/esp32/Light_PWM.hpp"

#include "board/board.hpp"
#include "diagnostics/diagnostics.hpp"

namespace display {
namespace {
class ClassicM5StackDisplay : public lgfx::LGFX_Device {
 public:
  bool ready() const {
    return getPanel() != nullptr;
  }

  protected:
  bool init_impl(bool use_reset, bool use_clear) override {
    diagnostics::log_line("LCD: init_impl enter");
    if (!setup_classic_m5stack_panel()) {
      diagnostics::log_line("LCD: setup failed");
      return false;
    }

    diagnostics::log_line("LCD: panel setup ok");
    this->setPanel(static_cast<lgfx::Panel_Device *>(&classic_panel()));
    auto *panel = this->getPanel();
    if (panel == nullptr) {
      diagnostics::log_line("LCD: panel null");
      return false;
    }

    diagnostics::log_line("LCD: panel init start");
    if (!panel->init(use_reset)) {
      diagnostics::log_line("LCD: panel init failed");
      return false;
    }
    diagnostics::log_line("LCD: panel init done");

    startWrite();
    invertDisplay(panel->getInvert());
    setColorDepth(panel->getWriteDepth());
    setRotation(panel->getRotation());
    if (use_clear) {
      clear();
    }
    setPivot(width() >> 1, height() >> 1);
    endWrite();
    setBrightness(255);
    panel->initTouch();
    diagnostics::log_line("LCD: panel init ok");
    return true;
  }

  private:
  static lgfx::Bus_SPI &classic_bus() {
    static lgfx::Bus_SPI bus;
    return bus;
  }

  class ClassicM5StackPanel : public lgfx::Panel_ILI9342 {
   public:
    ClassicM5StackPanel() {
      _cfg.pin_cs = GPIO_NUM_14;
      _cfg.pin_rst = GPIO_NUM_33;
      _cfg.offset_rotation = 3;
      _rotation = 1;
    }

    bool init(bool use_reset) override {
      _cfg.invert = lgfx::gpio::command(
        (const uint8_t[]) {
          lgfx::gpio::command_mode_output, GPIO_NUM_33,
          lgfx::gpio::command_write_low, GPIO_NUM_33,
          lgfx::gpio::command_mode_input_pulldown, GPIO_NUM_33,
          lgfx::gpio::command_write_high, GPIO_NUM_33,
          lgfx::gpio::command_read, GPIO_NUM_33,
          lgfx::gpio::command_mode_output, GPIO_NUM_33,
          lgfx::gpio::command_end
        });
      return lgfx::Panel_ILI9342::init(use_reset);
    }
  };

  static ClassicM5StackPanel &classic_panel() {
    static ClassicM5StackPanel lcd_panel;
    return lcd_panel;
  }

  static lgfx::Light_PWM &classic_backlight() {
    static lgfx::Light_PWM backlight;
    return backlight;
  }

  static bool setup_classic_m5stack_panel() {
    static bool configured = false;
    if (configured) {
      return true;
    }

    auto &bus = classic_bus();
    auto &lcd_panel = classic_panel();
    auto &backlight = classic_backlight();

    auto bus_cfg = bus.config();
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read = 16000000;
    bus_cfg.spi_mode = 0;
    bus_cfg.use_lock = true;
    bus_cfg.spi_host = VSPI_HOST;
    bus_cfg.dma_channel = 1;
    bus_cfg.pin_mosi = GPIO_NUM_23;
    bus_cfg.pin_miso = GPIO_NUM_19;
    bus_cfg.pin_sclk = GPIO_NUM_18;
    bus_cfg.pin_dc = GPIO_NUM_27;
    bus_cfg.spi_3wire = true;
    bus.config(bus_cfg);

    auto panel_cfg = lcd_panel.config();
    panel_cfg.pin_cs = GPIO_NUM_14;
    panel_cfg.pin_rst = GPIO_NUM_33;
    panel_cfg.offset_rotation = 3;
    panel_cfg.bus_shared = true;
    lcd_panel.config(panel_cfg);
    lcd_panel.setRotation(1);
    lcd_panel.setBus(&bus);

    auto light_cfg = backlight.config();
    light_cfg.pin_bl = GPIO_NUM_32;
    light_cfg.pwm_channel = 7;
    light_cfg.freq = 44100;
    light_cfg.offset = 0;
    light_cfg.invert = false;
    backlight.config(light_cfg);
    lcd_panel.setLight(&backlight);

    configured = true;
    return true;
  }
};

ClassicM5StackDisplay &display_device() {
  static ClassicM5StackDisplay device;
  return device;
}

bool display_ready() {
  return display_device().ready();
}
}  // namespace

void begin() {
  diagnostics::log_line("LCD: begin");
  auto &display = display_device();
  if (!display.init()) {
    diagnostics::log_line("LCD: init failed");
    return;
  }

  if (!display_ready()) {
    diagnostics::log_line("LCD: not ready");
    return;
  }

  board::I2cLock lock;
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.fillScreen(BLACK);
  diagnostics::log_line("LCD: manual ready");
}

bool is_narrow() {
  return display_ready() && display_device().width() < 200;
}

void show_boot_test() {
  show_text("LCD TEST", "SpoolSense");
}

void show_text(const char *line1, const char *line2) {
  if (!display_ready()) {
    return;
  }

  board::I2cLock lock;
  auto &display = display_device();
  display.fillScreen(BLACK);
  display.setCursor(0, 0);
  display.println(line1);
  if (line2 != nullptr) {
    display.println(line2);
  }
}

void append_line(const char *line) {
  if (!display_ready()) {
    return;
  }

  board::I2cLock lock;
  display_device().println(line);
}

void show_card_removed() {
  show_text("Card removed", "Waiting for card");
}

void show_uid_and_type(const uint8_t *uid, uint8_t uid_length) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char type_line[48] = {};
  snprintf(type_line, sizeof(type_line), "Type: %s", diagnostics::classify_tag_type(uid_length));

  show_text(uid_line, type_line);
}

void show_uid_and_usage(const uint8_t *uid, uint8_t uid_length, uint32_t usage_seconds,
                        uint8_t life_percent) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char usage_line[48] = {};
  snprintf(usage_line, sizeof(usage_line), "Usage: %lu s",
           static_cast<unsigned long>(usage_seconds));
  size_t used = strlen(usage_line);
  if (used + 1 < sizeof(usage_line)) {
    snprintf(usage_line + used, sizeof(usage_line) - used, " Life: %u%%",
             static_cast<unsigned>(life_percent));
  }

  show_text(uid_line, usage_line);
}

void show_page_read_failed() {
  append_line("Page 24: read failed");
}
}  // namespace display
