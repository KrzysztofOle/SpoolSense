/**
 * Native ESP-IDF LCD helper for the sandbox app.
 *
 * Features (EN):
 * - Configures the SPI bus and ILI9342 panel manually.
 * - Controls backlight and reset pins with native GPIO.
 * - Sends scanlines using esp_lcd.
 *
 * Funkcje (PL):
 * - Konfiguruje magistrale SPI i panel ILI9342 recznie.
 * - Steruje podswietleniem i resetem przez natywne GPIO.
 * - Wysyla linie obrazu przez esp_lcd.
 *
 * File: sandbox/espidf_native/main/src/native_lcd.cpp
 */

#include "sandbox/native_lcd.hpp"

#include <cstddef>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

namespace sandbox {
namespace {
constexpr gpio_num_t k_lcd_pin_mosi = GPIO_NUM_23;
constexpr gpio_num_t k_lcd_pin_miso = GPIO_NUM_19;
constexpr gpio_num_t k_lcd_pin_sclk = GPIO_NUM_18;
constexpr gpio_num_t k_lcd_pin_cs = GPIO_NUM_14;
constexpr gpio_num_t k_lcd_pin_dc = GPIO_NUM_27;
constexpr gpio_num_t k_lcd_pin_rst = GPIO_NUM_33;
constexpr gpio_num_t k_lcd_pin_backlight = GPIO_NUM_32;
constexpr spi_host_device_t k_lcd_host = SPI2_HOST;

SemaphoreHandle_t s_tx_done = nullptr;

bool lcd_wait_tx_done_impl(TickType_t timeout_ticks = pdMS_TO_TICKS(1000)) {
  return xSemaphoreTake(s_tx_done, timeout_ticks) == pdTRUE;
}

bool on_color_trans_done(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *) {
  BaseType_t higher_priority_task_woken = pdFALSE;
  xSemaphoreGiveFromISR(s_tx_done, &higher_priority_task_woken);
  return higher_priority_task_woken == pdTRUE;
}

esp_err_t lcd_write_cmd(esp_lcd_panel_io_handle_t io, uint8_t cmd) {
  return esp_lcd_panel_io_tx_param(io, cmd, nullptr, 0);
}

esp_err_t lcd_write_cmd(esp_lcd_panel_io_handle_t io, uint8_t cmd, const uint8_t *data, size_t len) {
  return esp_lcd_panel_io_tx_param(io, cmd, data, len);
}

void lcd_delay_ms(int ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
}

void lcd_reset_panel() {
  gpio_reset_pin(k_lcd_pin_rst);
  gpio_set_direction(k_lcd_pin_rst, GPIO_MODE_OUTPUT);
  gpio_set_level(k_lcd_pin_rst, 0);
  lcd_delay_ms(20);
  gpio_set_level(k_lcd_pin_rst, 1);
  lcd_delay_ms(120);
}

void lcd_enable_backlight() {
  gpio_reset_pin(k_lcd_pin_backlight);
  gpio_set_direction(k_lcd_pin_backlight, GPIO_MODE_OUTPUT);
  gpio_set_level(k_lcd_pin_backlight, 1);
}
}  // namespace

esp_err_t NativeLcd::begin() {
  if (s_tx_done == nullptr) {
    s_tx_done = xSemaphoreCreateBinary();
    if (s_tx_done == nullptr) {
      return ESP_ERR_NO_MEM;
    }
  }

  lcd_enable_backlight();
  lcd_reset_panel();

  spi_bus_config_t buscfg = {};
  buscfg.sclk_io_num = k_lcd_pin_sclk;
  buscfg.mosi_io_num = k_lcd_pin_mosi;
  buscfg.miso_io_num = k_lcd_pin_miso;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = kWidth * 2;

  ESP_RETURN_ON_ERROR(spi_bus_initialize(k_lcd_host, &buscfg, SPI_DMA_CH_AUTO), "sandbox",
                      "spi_bus_initialize failed");

  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.cs_gpio_num = k_lcd_pin_cs;
  io_config.dc_gpio_num = k_lcd_pin_dc;
  io_config.spi_mode = 0;
  io_config.pclk_hz = 40 * 1000 * 1000;
  io_config.trans_queue_depth = 1;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;
  io_config.on_color_trans_done = on_color_trans_done;
  io_config.user_ctx = nullptr;

  ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)k_lcd_host, &io_config, &io_handle_),
                      "sandbox", "esp_lcd_new_panel_io_spi failed");

  static const uint8_t set_extc[] = {0xFF, 0x93, 0x42};
  static const uint8_t pwctr1[] = {0x12, 0x12};
  static const uint8_t pwctr2[] = {0x03};
  static const uint8_t vmctr1[] = {0xF2};
  static const uint8_t gamma_pos[] = {0x00, 0x0C, 0x11, 0x04, 0x11, 0x08, 0x37, 0x89, 0x4C, 0x06,
                                      0x0C, 0x0A, 0x2E, 0x34, 0x0F};
  static const uint8_t gamma_neg[] = {0x00, 0x0B, 0x11, 0x05, 0x13, 0x09, 0x33, 0x67, 0x48, 0x07,
                                      0x0E, 0x0B, 0x2E, 0x33, 0x0F};
  static const uint8_t dfunctr[] = {0x08, 0x82, 0x1D, 0x04};
  static const uint8_t madctl[] = {0x08};
  static const uint8_t colmod[] = {0x55};
  static const uint8_t pwr_ctrl[] = {0x01, 0x00, 0x00};

  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x01), "sandbox", "SWRESET failed");
  lcd_delay_ms(120);
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x11), "sandbox", "SLPOUT failed");
  lcd_delay_ms(120);
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xFF, set_extc, sizeof(set_extc)), "sandbox", "SETEXTC failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC0, pwctr1, sizeof(pwctr1)), "sandbox", "PWCTR1 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC1, pwctr2, sizeof(pwctr2)), "sandbox", "PWCTR2 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC5, vmctr1, sizeof(vmctr1)), "sandbox", "VMCTR1 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xB0, (const uint8_t *)"\xE0", 1), "sandbox", "B0 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xF6, pwr_ctrl, sizeof(pwr_ctrl)), "sandbox", "F6 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xE0, gamma_pos, sizeof(gamma_pos)), "sandbox", "GMCTRP1 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xE1, gamma_neg, sizeof(gamma_neg)), "sandbox", "GMCTRN1 failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xB6, dfunctr, sizeof(dfunctr)), "sandbox", "DFUNCTR failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x3A, colmod, sizeof(colmod)), "sandbox", "COLMOD failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x36, madctl, sizeof(madctl)), "sandbox", "MADCTL failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x29), "sandbox", "DISPON failed");
  lcd_delay_ms(20);

  return ESP_OK;
}

esp_err_t NativeLcd::send_line(int y, const uint16_t *line) {
  uint8_t column_data[4];
  column_data[0] = 0;
  column_data[1] = 0;
  column_data[2] = static_cast<uint8_t>((kWidth - 1) >> 8);
  column_data[3] = static_cast<uint8_t>((kWidth - 1) & 0xFF);

  uint8_t row_data[4];
  row_data[0] = static_cast<uint8_t>(y >> 8);
  row_data[1] = static_cast<uint8_t>(y & 0xFF);
  row_data[2] = static_cast<uint8_t>(y >> 8);
  row_data[3] = static_cast<uint8_t>(y & 0xFF);

  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x2A, column_data, sizeof(column_data)), "sandbox",
                      "CASET failed");
  ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x2B, row_data, sizeof(row_data)), "sandbox", "PASET failed");
  return esp_lcd_panel_io_tx_color(io_handle_, 0x2C, line, kWidth * sizeof(uint16_t));
}

bool NativeLcd::wait_tx_done() {
  return lcd_wait_tx_done_impl();
}
}  // namespace sandbox
