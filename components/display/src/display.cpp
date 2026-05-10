/**
 * Native LCD helpers for the firmware UI.
 *
 * Features (EN):
 * - Initializes the M5Stack Core LCD directly through esp_lcd.
 * - Renders the firmware status screen with LVGL labels.
 * - Keeps the display path independent from Arduino display helpers.
 *
 * Funkcje (PL):
 * - Inicjalizuje LCD M5Stack Core bezposrednio przez esp_lcd.
 * - Rysuje ekran statusu firmware za pomoca etykiet LVGL.
 * - Uniezaleznia obsluge wyswietlacza od pomocniczych bibliotek Arduino.
 *
 * File: components/display/src/display.cpp
 */

#include "display/display.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "diagnostics/diagnostics.hpp"

namespace display {
namespace {
enum class ColorOrder : uint8_t {
  kRgb = 0,
  kBgr = 1,
};

constexpr gpio_num_t k_lcd_pin_mosi = GPIO_NUM_23;
constexpr gpio_num_t k_lcd_pin_miso = GPIO_NUM_19;
constexpr gpio_num_t k_lcd_pin_sclk = GPIO_NUM_18;
constexpr gpio_num_t k_lcd_pin_cs = GPIO_NUM_14;
constexpr gpio_num_t k_lcd_pin_dc = GPIO_NUM_27;
constexpr gpio_num_t k_lcd_pin_rst = GPIO_NUM_33;
constexpr gpio_num_t k_lcd_pin_backlight = GPIO_NUM_32;
constexpr spi_host_device_t k_lcd_host = SPI2_HOST;

constexpr lv_coord_t k_line_x = 12;
constexpr lv_coord_t k_title_bar_x = 0;
constexpr lv_coord_t k_title_bar_y = 0;
constexpr lv_coord_t k_title_bar_h = 34;
constexpr lv_coord_t k_footer_y = 206;
constexpr lv_coord_t k_footer_h = 30;
constexpr lv_coord_t k_footer_gap = 8;
constexpr lv_coord_t k_regular_line_y[] = {10, 44, 78, 112};
constexpr lv_coord_t k_diagnostic_line_y[] = {10, 56, 102, 148};
constexpr lv_coord_t k_home_title_y = 8;
constexpr lv_coord_t k_home_line_1_y = 38;
constexpr lv_coord_t k_home_line_2_y = 68;
constexpr lv_coord_t k_home_line_3_y = 98;
constexpr lv_coord_t k_home_line_4_y = 128;
constexpr lv_coord_t k_home_bar_x = 12;
constexpr lv_coord_t k_home_bar_y = 150;
constexpr lv_coord_t k_home_bar_w = 296;
constexpr lv_coord_t k_home_bar_h = 24;
constexpr lv_coord_t k_home_bar_text_y = 178;
constexpr lv_coord_t k_diag_line_1_y = 46;
constexpr lv_coord_t k_diag_line_2_y = 82;
constexpr lv_coord_t k_diag_line_3_y = 118;
constexpr lv_coord_t k_diag_line_4_y = 154;
constexpr lv_coord_t k_scale_line_1_y = 56;
constexpr lv_coord_t k_scale_line_2_y = 96;
constexpr lv_coord_t k_scale_line_3_y = 160;
constexpr lv_coord_t k_rfid_line_1_y = 50;
constexpr lv_coord_t k_rfid_line_2_y = 84;
constexpr lv_coord_t k_rfid_line_3_y = 118;
constexpr lv_coord_t k_rfid_line_4_y = 152;
constexpr char k_empty_line[] = "";
constexpr size_t k_max_line_length = 128;
constexpr int k_draw_buffer_lines = 30;

SemaphoreHandle_t s_tx_done = nullptr;
bool s_bus_initialized = false;

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

uint8_t madctl_for_order(ColorOrder order) {
  switch (order) {
    case ColorOrder::kRgb:
      return 0x00;
    case ColorOrder::kBgr:
      return 0x08;
  }

  return 0x00;
}

uint32_t millis_now() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
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

class NativeLcd {
 public:
  static constexpr int kWidth = 320;
  static constexpr int kHeight = 240;

  esp_err_t begin() {
    if (started_) {
      return ESP_OK;
    }

    if (s_tx_done == nullptr) {
      s_tx_done = xSemaphoreCreateBinary();
      if (s_tx_done == nullptr) {
        return ESP_ERR_NO_MEM;
      }
    }

    lcd_enable_backlight();
    lcd_reset_panel();

    if (!s_bus_initialized) {
      spi_bus_config_t buscfg = {};
      buscfg.sclk_io_num = k_lcd_pin_sclk;
      buscfg.mosi_io_num = k_lcd_pin_mosi;
      buscfg.miso_io_num = k_lcd_pin_miso;
      buscfg.quadwp_io_num = -1;
      buscfg.quadhd_io_num = -1;
      buscfg.max_transfer_sz = kWidth * 40 * 2;

      ESP_RETURN_ON_ERROR(spi_bus_initialize(k_lcd_host, &buscfg, SPI_DMA_CH_AUTO), "display",
                          "spi_bus_initialize failed");
      s_bus_initialized = true;
    }

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

    ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)k_lcd_host, &io_config, &io_handle_),
      "display", "esp_lcd_new_panel_io_spi failed");

    static const uint8_t set_extc[] = {0xFF, 0x93, 0x42};
    static const uint8_t pwctr1[] = {0x12, 0x12};
    static const uint8_t pwctr2[] = {0x03};
    static const uint8_t vmctr1[] = {0xF2};
    static const uint8_t gamma_pos[] = {0x00, 0x0C, 0x11, 0x04, 0x11, 0x08, 0x37, 0x89, 0x4C, 0x06,
                                        0x0C, 0x0A, 0x2E, 0x34, 0x0F};
    static const uint8_t gamma_neg[] = {0x00, 0x0B, 0x11, 0x05, 0x13, 0x09, 0x33, 0x67, 0x48, 0x07,
                                        0x0E, 0x0B, 0x2E, 0x33, 0x0F};
    static const uint8_t dfunctr[] = {0x08, 0x82, 0x1D, 0x04};
    static const uint8_t colmod[] = {0x66};
    static const uint8_t pwr_ctrl[] = {0x01, 0x00, 0x00};

    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x01), "display", "SWRESET failed");
    lcd_delay_ms(120);
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x11), "display", "SLPOUT failed");
    lcd_delay_ms(120);
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xFF, set_extc, sizeof(set_extc)), "display",
                        "SETEXTC failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC0, pwctr1, sizeof(pwctr1)), "display",
                        "PWCTR1 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC1, pwctr2, sizeof(pwctr2)), "display",
                        "PWCTR2 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xC5, vmctr1, sizeof(vmctr1)), "display",
                        "VMCTR1 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xB0, (const uint8_t *)"\xE0", 1), "display",
                        "B0 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xF6, pwr_ctrl, sizeof(pwr_ctrl)), "display",
                        "F6 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xE0, gamma_pos, sizeof(gamma_pos)), "display",
                        "GMCTRP1 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xE1, gamma_neg, sizeof(gamma_neg)), "display",
                        "GMCTRN1 failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0xB6, dfunctr, sizeof(dfunctr)), "display",
                        "DFUNCTR failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x3A, colmod, sizeof(colmod)), "display",
                        "COLMOD failed");
    ESP_RETURN_ON_ERROR(set_color_order(color_order_), "display", "MADCTL failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x21), "display", "INVON failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x29), "display", "DISPON failed");
    lcd_delay_ms(20);

    started_ = true;
    return ESP_OK;
  }

  esp_err_t set_color_order(ColorOrder order) {
    color_order_ = order;
    const uint8_t madctl = madctl_for_order(order);
    return lcd_write_cmd(io_handle_, 0x36, &madctl, 1);
  }

  esp_err_t send_area(int x1, int y1, int x2, int y2, const uint8_t *pixels) {
    uint8_t column_data[4];
    column_data[0] = static_cast<uint8_t>(x1 >> 8);
    column_data[1] = static_cast<uint8_t>(x1 & 0xFF);
    column_data[2] = static_cast<uint8_t>(x2 >> 8);
    column_data[3] = static_cast<uint8_t>(x2 & 0xFF);

    uint8_t row_data[4];
    row_data[0] = static_cast<uint8_t>(y1 >> 8);
    row_data[1] = static_cast<uint8_t>(y1 & 0xFF);
    row_data[2] = static_cast<uint8_t>(y2 >> 8);
    row_data[3] = static_cast<uint8_t>(y2 & 0xFF);

    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x2A, column_data, sizeof(column_data)), "display",
                        "CASET failed");
    ESP_RETURN_ON_ERROR(lcd_write_cmd(io_handle_, 0x2B, row_data, sizeof(row_data)), "display",
                        "PASET failed");

    const int width = x2 - x1 + 1;
    const int height = y2 - y1 + 1;
    const size_t payload_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 3U;
    return esp_lcd_panel_io_tx_color(io_handle_, 0x2C, pixels, payload_size);
  }

  bool wait_tx_done() {
    return lcd_wait_tx_done_impl();
  }

  int width() const {
    return kWidth;
  }

 private:
  ColorOrder color_order_ = ColorOrder::kRgb;
  esp_lcd_panel_io_handle_t io_handle_ = nullptr;
  bool started_ = false;
};

void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  auto *lcd = static_cast<NativeLcd *>(disp_drv->user_data);
  if (lcd == nullptr) {
    lv_disp_flush_ready(disp_drv);
    return;
  }

  const int x1 = area->x1;
  const int y1 = area->y1;
  const int x2 = area->x2;
  const int y2 = area->y2;
  const int width = (x2 - x1 + 1);
  const int height = (y2 - y1 + 1);
  const int px_count = width * height;

  static uint8_t color_buffer[NativeLcd::kWidth * k_draw_buffer_lines * 3];
  for (int i = 0; i < px_count; ++i) {
    const uint16_t color = color_p[i].full;
    const uint8_t r5 = static_cast<uint8_t>((color >> 11) & 0x1FU);
    const uint8_t g6 = static_cast<uint8_t>((color >> 5) & 0x3FU);
    const uint8_t b5 = static_cast<uint8_t>(color & 0x1FU);
    const uint8_t r6 = static_cast<uint8_t>((r5 << 1U) | (r5 >> 4U));
    const uint8_t b6 = static_cast<uint8_t>((b5 << 1U) | (b5 >> 4U));
    const size_t offset = static_cast<size_t>(i) * 3U;
    color_buffer[offset + 0] = static_cast<uint8_t>(r6 << 2U);
    color_buffer[offset + 1] = static_cast<uint8_t>(g6 << 2U);
    color_buffer[offset + 2] = static_cast<uint8_t>(b6 << 2U);
  }

  if (lcd->send_area(x1, y1, x2, y2, color_buffer) != ESP_OK) {
    diagnostics::log_line("LCD: flush failed");
  } else if (!lcd->wait_tx_done()) {
    diagnostics::log_line("LCD: flush timeout");
  }

  lv_disp_flush_ready(disp_drv);
}

class NativeDisplay {
 public:
  enum class ScreenStyle : uint8_t {
    kRegular,
    kDiagnostic,
    kHome,
    kDiagnostics,
    kScale,
    kRfid,
  };

  bool begin() {
    if (configured_) {
      return true;
    }

    if (lcd_.begin() != ESP_OK) {
      diagnostics::log_line("LCD: native init failed");
      return false;
    }

    if (lcd_.set_color_order(ColorOrder::kBgr) != ESP_OK) {
      diagnostics::log_line("LCD: color order failed");
      return false;
    }

    lv_init();
    lv_disp_draw_buf_init(&draw_buf_, draw_buffer_, nullptr, NativeLcd::kWidth * k_draw_buffer_lines);

    lv_disp_drv_init(&disp_drv_);
    disp_drv_.hor_res = NativeLcd::kWidth;
    disp_drv_.ver_res = NativeLcd::kHeight;
    disp_drv_.flush_cb = flush_cb;
    disp_drv_.draw_buf = &draw_buf_;
    disp_drv_.user_data = &lcd_;
    disp_ = lv_disp_drv_register(&disp_drv_);
    if (disp_ == nullptr) {
      diagnostics::log_line("LCD: LVGL register failed");
      return false;
    }
    lv_disp_set_default(disp_);

    clear_lines();
    build_screen(ScreenStyle::kRegular);
    sync_time();
    lv_timer_handler();
    configured_ = true;
    return true;
  }

  bool is_narrow() const {
    return configured_ && lcd_.width() < 200;
  }

  void show_boot_test() {
    show_text("LCD TEST", "SpoolSense");
  }

  void show_text(const char *line1, const char *line2) {
    show_lines(line1, line2, nullptr, nullptr);
  }

  void show_lines(const char *line1, const char *line2, const char *line3, const char *line4) {
    show_lines_internal(ScreenStyle::kRegular, line1, line2, line3, line4);
  }

  void show_diagnostics(const char *line1, const char *line2, const char *line3, const char *line4) {
    show_lines_internal(ScreenStyle::kDiagnostic, line1, line2, line3, line4);
  }

  void render_home(const HomeSnapshot &snapshot) {
    UiSnapshot ui_snapshot{};
    ui_snapshot.screen = UiScreen::kHome;
    ui_snapshot.home = snapshot;
    render(ui_snapshot);
  }

  void render(const UiSnapshot &snapshot) {
    if (!configured_) {
      return;
    }

    if (has_ui_snapshot_ && is_same_ui_snapshot(snapshot, last_ui_snapshot_)) {
      ui_dirty_ = false;
      return;
    }

    last_ui_snapshot_ = snapshot;
    has_ui_snapshot_ = true;
    ui_dirty_ = true;

    switch (snapshot.screen) {
      case UiScreen::kHome:
        ensure_style(ScreenStyle::kHome);
        refresh_home(snapshot.home);
        break;
      case UiScreen::kDiagnostics:
        ensure_style(ScreenStyle::kDiagnostics);
        refresh_diagnostics(snapshot.diagnostics);
        break;
      case UiScreen::kScale:
        ensure_style(ScreenStyle::kScale);
        refresh_scale(snapshot.scale);
        break;
      case UiScreen::kRfid:
        ensure_style(ScreenStyle::kRfid);
        refresh_rfid(snapshot.rfid);
        break;
      default:
        break;
    }
  }

  void append_line(const char *line) {
    if (!configured_ || line == nullptr) {
      return;
    }

    const size_t index = line_count_ < 4U ? line_count_ : 3U;
    set_line(index, line);
    if (line_count_ < 4U) {
      ++line_count_;
    }
    refresh();
  }

 private:
  void ensure_style(ScreenStyle style) {
    if (configured_ && style == screen_style_) {
      return;
    }

    screen_style_ = style;
    build_screen(style);
  }

  void show_lines_internal(ScreenStyle style, const char *line1, const char *line2, const char *line3,
                           const char *line4) {
    if (!configured_) {
      return;
    }

    ensure_style(style);
    clear_lines();
    set_line(0, line1);
    set_line(1, line2);
    set_line(2, line3);
    set_line(3, line4);
    line_count_ = 4U;
    refresh();
  }

  void clear_lines() {
    for (auto &line : lines_) {
      line[0] = '\0';
    }
    line_count_ = 0;
  }

  void set_line(size_t index, const char *text) {
    if (index >= lines_.size()) {
      return;
    }

    if (text == nullptr) {
      lines_[index][0] = '\0';
      return;
    }

    std::snprintf(lines_[index].data(), lines_[index].size(), "%s", text);
  }

  void build_screen(ScreenStyle style) {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    if (style == ScreenStyle::kHome) {
      build_home_screen(scr);
      return;
    }
    if (style == ScreenStyle::kDiagnostics) {
      build_diagnostics_screen(scr);
      return;
    }
    if (style == ScreenStyle::kScale) {
      build_scale_screen(scr);
      return;
    }
    if (style == ScreenStyle::kRfid) {
      build_rfid_screen(scr);
      return;
    }

    const lv_coord_t *line_y = style == ScreenStyle::kDiagnostic ? k_diagnostic_line_y : k_regular_line_y;
    const lv_font_t *font = style == ScreenStyle::kDiagnostic ? &lv_font_montserrat_20 : &lv_font_montserrat_14;

    for (size_t index = 0; index < labels_.size(); ++index) {
      labels_[index] = lv_label_create(scr);
      lv_obj_set_style_text_color(labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(labels_[index], font, LV_PART_MAIN);
      lv_obj_set_style_text_align(labels_[index], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
      lv_label_set_long_mode(labels_[index], LV_LABEL_LONG_WRAP);
      lv_obj_set_width(labels_[index], NativeLcd::kWidth - 24);
      lv_obj_set_pos(labels_[index], k_line_x, line_y[index]);
    }
  }

  void refresh() {
    sync_time();

    for (size_t index = 0; index < labels_.size(); ++index) {
      lv_label_set_text(labels_[index], lines_[index][0] == '\0' ? k_empty_line : lines_[index].data());
    }

    lv_timer_handler();
    lv_refr_now(nullptr);
  }

  void build_home_screen(lv_obj_t *scr) {
    home_title_bar_ = lv_obj_create(scr);
    configure_title_bar(home_title_bar_);
    home_title_label_ = lv_label_create(home_title_bar_);
    configure_title_label(home_title_label_);
    lv_obj_set_style_text_font(home_title_label_, &lv_font_montserrat_22, LV_PART_MAIN);

    for (size_t index = 0; index < home_info_labels_.size(); ++index) {
      home_info_labels_[index] = lv_label_create(scr);
      lv_obj_set_style_text_color(home_info_labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(home_info_labels_[index],
                                 index < 3 ? &lv_font_montserrat_20 : &lv_font_montserrat_14,
                                 LV_PART_MAIN);
      lv_obj_set_style_text_align(home_info_labels_[index], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
      lv_obj_set_width(home_info_labels_[index], NativeLcd::kWidth - 24);
      lv_obj_set_pos(home_info_labels_[index], k_line_x,
                     index == 0 ? k_home_line_1_y
                                : (index == 1 ? k_home_line_2_y : (index == 2 ? k_home_line_3_y : k_home_line_4_y)));
    }

    home_bar_bg_ = lv_obj_create(scr);
    lv_obj_remove_style_all(home_bar_bg_);
    lv_obj_set_pos(home_bar_bg_, k_home_bar_x, k_home_bar_y);
    lv_obj_set_size(home_bar_bg_, k_home_bar_w, k_home_bar_h);
    lv_obj_set_style_bg_color(home_bar_bg_, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(home_bar_bg_, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(home_bar_bg_, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_set_style_border_width(home_bar_bg_, 1, LV_PART_MAIN);

    home_bar_fill_ = lv_obj_create(scr);
    lv_obj_remove_style_all(home_bar_fill_);
    lv_obj_set_pos(home_bar_fill_, k_home_bar_x + 1, k_home_bar_y + 1);
    lv_obj_set_size(home_bar_fill_, 0, k_home_bar_h - 2);
    lv_obj_set_style_bg_color(home_bar_fill_, lv_color_hex(0x2ECC71), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(home_bar_fill_, LV_OPA_COVER, LV_PART_MAIN);

    home_remain_label_ = lv_label_create(scr);
    lv_obj_set_style_text_color(home_remain_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(home_remain_label_, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(home_remain_label_, k_line_x, k_home_bar_text_y);

    build_footer(scr);
  }

  void refresh_home(const HomeSnapshot &snapshot) {
    if (!ui_dirty_) {
      return;
    }

    char title[48] = {};
    std::snprintf(title, sizeof(title), "SpoolSense");
    lv_label_set_text(home_title_label_, title);

    char line1[64] = {};
    std::snprintf(line1, sizeof(line1), "%s / %s",
                  snapshot.material[0] == '\0' ? "-" : snapshot.material,
                  snapshot.color[0] == '\0' ? "-" : snapshot.color);
    lv_label_set_text(home_info_labels_[0], line1);

    char line2[96] = {};
    std::snprintf(line2, sizeof(line2), "W:%ldg  R:%ldg  U:%ldg",
                  static_cast<long>(snapshot.current_weight_g),
                  static_cast<long>(snapshot.reference_full_weight_g),
                  static_cast<long>(snapshot.used_weight_g));
    lv_label_set_text(home_info_labels_[1], line2);

    lv_label_set_text(home_info_labels_[2], "");

    lv_label_set_text(home_info_labels_[3], "");

    uint8_t percent = snapshot.remaining_percent;
    if (percent > 100U) {
      percent = 100U;
    }
    const lv_coord_t inner_w = k_home_bar_w - 2;
    const lv_coord_t fill_w = static_cast<lv_coord_t>((static_cast<int32_t>(inner_w) * percent) / 100);
    lv_obj_set_size(home_bar_fill_, fill_w, k_home_bar_h - 2);

    char remain_line[48] = {};
    std::snprintf(remain_line, sizeof(remain_line), "Remain: %u%%", static_cast<unsigned>(percent));
    lv_label_set_text(home_remain_label_, remain_line);
    refresh_footer("PREV", "-", "NEXT");

    lv_timer_handler();
    lv_refr_now(nullptr);
    ui_dirty_ = false;
  }

  void build_diagnostics_screen(lv_obj_t *scr) {
    diagnostics_title_bar_ = lv_obj_create(scr);
    configure_title_bar(diagnostics_title_bar_);
    diagnostics_title_label_ = lv_label_create(diagnostics_title_bar_);
    configure_title_label(diagnostics_title_label_);
    lv_obj_set_style_text_font(diagnostics_title_label_, &lv_font_montserrat_20, LV_PART_MAIN);

    for (size_t index = 0; index < diagnostics_labels_.size(); ++index) {
      diagnostics_labels_[index] = lv_label_create(scr);
      lv_obj_set_style_text_color(diagnostics_labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(diagnostics_labels_[index], &lv_font_montserrat_20, LV_PART_MAIN);
      lv_obj_set_style_text_align(diagnostics_labels_[index], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
      lv_obj_set_width(diagnostics_labels_[index], NativeLcd::kWidth - 24);
      const lv_coord_t y = index == 0 ? k_diag_line_1_y
                           : index == 1 ? k_diag_line_2_y
                           : index == 2 ? k_diag_line_3_y
                                        : k_diag_line_4_y;
      lv_obj_set_pos(diagnostics_labels_[index], k_line_x, y);
    }

    build_footer(scr);
  }

  void refresh_diagnostics(const DiagnosticsSnapshot &snapshot) {
    if (!ui_dirty_) {
      return;
    }

    lv_label_set_text(diagnostics_title_label_, "Diagnostics");
    char line[48] = {};
    std::snprintf(line, sizeof(line), "HX711     %s", health_text(snapshot.hx711));
    lv_label_set_text(diagnostics_labels_[0], line);
    std::snprintf(line, sizeof(line), "PN532     %s", health_text(snapshot.pn532));
    lv_label_set_text(diagnostics_labels_[1], line);
    std::snprintf(line, sizeof(line), "DISPLAY   %s", health_text(snapshot.display));
    lv_label_set_text(diagnostics_labels_[2], line);
    std::snprintf(line, sizeof(line), "I2C       %s", health_text(snapshot.i2c));
    lv_label_set_text(diagnostics_labels_[3], line);
    refresh_footer("PREV", "-", "NEXT");

    lv_timer_handler();
    lv_refr_now(nullptr);
    ui_dirty_ = false;
  }

  void build_scale_screen(lv_obj_t *scr) {
    scale_title_bar_ = lv_obj_create(scr);
    configure_title_bar(scale_title_bar_);
    scale_title_label_ = lv_label_create(scale_title_bar_);
    configure_title_label(scale_title_label_);
    lv_obj_set_style_text_font(scale_title_label_, &lv_font_montserrat_20, LV_PART_MAIN);

    for (size_t index = 0; index < scale_labels_.size(); ++index) {
      scale_labels_[index] = lv_label_create(scr);
      lv_obj_set_style_text_color(scale_labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(scale_labels_[index], &lv_font_montserrat_20, LV_PART_MAIN);
      lv_obj_set_style_text_align(scale_labels_[index], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
      lv_obj_set_width(scale_labels_[index], NativeLcd::kWidth - 24);
      const lv_coord_t y = index == 0 ? k_scale_line_1_y : (index == 1 ? k_scale_line_2_y : k_scale_line_3_y);
      lv_obj_set_pos(scale_labels_[index], k_line_x, y);
    }

    build_footer(scr);
  }

  void refresh_scale(const ScaleSnapshot &snapshot) {
    if (!ui_dirty_) {
      return;
    }

    lv_label_set_text(scale_title_label_, "Scale");
    char line[64] = {};
    std::snprintf(line, sizeof(line), "Weight: %ld g", static_cast<long>(snapshot.current_weight_g));
    lv_label_set_text(scale_labels_[0], line);
    std::snprintf(line, sizeof(line), "Ref:    %ld g", static_cast<long>(snapshot.reference_full_weight_g));
    lv_label_set_text(scale_labels_[1], line);
    if (snapshot.status_message[0] != '\0') {
      lv_label_set_text(scale_labels_[2], snapshot.status_message);
    } else {
      lv_label_set_text(scale_labels_[2], "");
    }
    refresh_footer("TARA", "SAVE REF", "NEXT");

    lv_timer_handler();
    lv_refr_now(nullptr);
    ui_dirty_ = false;
  }

  void build_rfid_screen(lv_obj_t *scr) {
    rfid_title_bar_ = lv_obj_create(scr);
    configure_title_bar(rfid_title_bar_);
    rfid_title_label_ = lv_label_create(rfid_title_bar_);
    configure_title_label(rfid_title_label_);
    lv_obj_set_style_text_font(rfid_title_label_, &lv_font_montserrat_20, LV_PART_MAIN);

    for (size_t index = 0; index < rfid_labels_.size(); ++index) {
      rfid_labels_[index] = lv_label_create(scr);
      lv_obj_set_style_text_color(rfid_labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(rfid_labels_[index], &lv_font_montserrat_20, LV_PART_MAIN);
      lv_obj_set_style_text_align(rfid_labels_[index], LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
      lv_obj_set_width(rfid_labels_[index], NativeLcd::kWidth - 24);
      const lv_coord_t y = index == 0   ? k_rfid_line_1_y
                           : index == 1 ? k_rfid_line_2_y
                           : index == 2 ? k_rfid_line_3_y
                                        : k_rfid_line_4_y;
      lv_obj_set_pos(rfid_labels_[index], k_line_x, y);
    }

    build_footer(scr);
  }

  void refresh_rfid(const RfidSnapshot &snapshot) {
    if (!ui_dirty_) {
      return;
    }

    lv_label_set_text(rfid_title_label_, "RFID");
    if (!snapshot.card_present) {
      lv_label_set_text(rfid_labels_[0], "NO CARD");
      lv_label_set_text(rfid_labels_[1], "PLACE TAG");
      lv_label_set_text(rfid_labels_[2], "");
      lv_label_set_text(rfid_labels_[3], "");
    } else {
      char line[80] = {};
      std::snprintf(line, sizeof(line), "UID: %s", snapshot.uid[0] == '\0' ? "-" : snapshot.uid);
      lv_label_set_text(rfid_labels_[0], line);
      std::snprintf(line, sizeof(line), "Material: %s", snapshot.material[0] == '\0' ? "-" : snapshot.material);
      lv_label_set_text(rfid_labels_[1], line);
      if (snapshot.status_message[0] != '\0') {
        std::snprintf(line, sizeof(line), "%s", snapshot.status_message);
      } else {
        std::snprintf(line, sizeof(line), "Color: %s", snapshot.color[0] == '\0' ? "-" : snapshot.color);
      }
      lv_label_set_text(rfid_labels_[2], line);
      std::snprintf(line, sizeof(line), "Ref: %ld g", static_cast<long>(snapshot.reference_full_weight_g));
      lv_label_set_text(rfid_labels_[3], line);
    }
    refresh_footer("PREV", "SAVE TAG", "NEXT");

    lv_timer_handler();
    lv_refr_now(nullptr);
    ui_dirty_ = false;
  }

  static const char *health_text(uint8_t health) {
    switch (health) {
      case 2:
        return "OK";
      case 3:
        return "WARN";
      case 4:
        return "ERR";
      case 5:
        return "MISS";
      case 1:
        return "INIT";
      case 0:
      default:
        return "UNK";
    }
  }

  static bool is_same_home_snapshot(const HomeSnapshot &lhs, const HomeSnapshot &rhs) {
    return std::memcmp(lhs.material, rhs.material, sizeof(lhs.material)) == 0 &&
           std::memcmp(lhs.color, rhs.color, sizeof(lhs.color)) == 0 &&
           lhs.current_weight_g == rhs.current_weight_g &&
           lhs.reference_full_weight_g == rhs.reference_full_weight_g &&
           lhs.used_weight_g == rhs.used_weight_g &&
           lhs.remaining_percent == rhs.remaining_percent;
  }

  static bool is_same_diagnostics_snapshot(const DiagnosticsSnapshot &lhs, const DiagnosticsSnapshot &rhs) {
    return lhs.hx711 == rhs.hx711 && lhs.pn532 == rhs.pn532 && lhs.display == rhs.display &&
           lhs.i2c == rhs.i2c;
  }

  static bool is_same_scale_snapshot(const ScaleSnapshot &lhs, const ScaleSnapshot &rhs) {
    return lhs.current_weight_g == rhs.current_weight_g &&
           lhs.reference_full_weight_g == rhs.reference_full_weight_g &&
           std::memcmp(lhs.status_message, rhs.status_message, sizeof(lhs.status_message)) == 0;
  }

  static bool is_same_rfid_snapshot(const RfidSnapshot &lhs, const RfidSnapshot &rhs) {
    return lhs.card_present == rhs.card_present &&
           std::memcmp(lhs.uid, rhs.uid, sizeof(lhs.uid)) == 0 &&
           std::memcmp(lhs.material, rhs.material, sizeof(lhs.material)) == 0 &&
           std::memcmp(lhs.color, rhs.color, sizeof(lhs.color)) == 0 &&
           lhs.reference_full_weight_g == rhs.reference_full_weight_g &&
           std::memcmp(lhs.status_message, rhs.status_message, sizeof(lhs.status_message)) == 0;
  }

  static bool is_same_ui_snapshot(const UiSnapshot &lhs, const UiSnapshot &rhs) {
    if (lhs.screen != rhs.screen) {
      return false;
    }

    switch (lhs.screen) {
      case UiScreen::kHome:
        return is_same_home_snapshot(lhs.home, rhs.home);
      case UiScreen::kDiagnostics:
        return is_same_diagnostics_snapshot(lhs.diagnostics, rhs.diagnostics);
      case UiScreen::kScale:
        return is_same_scale_snapshot(lhs.scale, rhs.scale);
      case UiScreen::kRfid:
        return is_same_rfid_snapshot(lhs.rfid, rhs.rfid);
      default:
        return false;
    }
  }

  void configure_title_bar(lv_obj_t *bar) {
    lv_obj_remove_style_all(bar);
    lv_obj_set_pos(bar, k_title_bar_x, k_title_bar_y);
    lv_obj_set_size(bar, NativeLcd::kWidth, k_title_bar_h);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x1E2A36), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(bar, lv_color_hex(0x2F4254), LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
  }

  void configure_title_label(lv_obj_t *label) {
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(label, NativeLcd::kWidth);
    lv_obj_set_pos(label, 0, k_home_title_y);
  }

  void build_footer(lv_obj_t *scr) {
    const lv_coord_t total_gap = 2 * k_footer_gap;
    const lv_coord_t box_w = (NativeLcd::kWidth - 2 * k_line_x - total_gap) / 3;

    for (size_t index = 0; index < footer_boxes_.size(); ++index) {
      footer_boxes_[index] = lv_obj_create(scr);
      lv_obj_remove_style_all(footer_boxes_[index]);
      lv_obj_set_size(footer_boxes_[index], box_w, k_footer_h);
      lv_obj_set_pos(footer_boxes_[index], k_line_x + static_cast<lv_coord_t>(index) * (box_w + k_footer_gap),
                     k_footer_y);
      lv_obj_set_style_bg_color(footer_boxes_[index], lv_color_hex(0x243344), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(footer_boxes_[index], LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_border_color(footer_boxes_[index], lv_color_hex(0x4F6B86), LV_PART_MAIN);
      lv_obj_set_style_border_width(footer_boxes_[index], 1, LV_PART_MAIN);

      footer_labels_[index] = lv_label_create(footer_boxes_[index]);
      lv_obj_set_style_text_color(footer_labels_[index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_text_font(footer_labels_[index], &lv_font_montserrat_14, LV_PART_MAIN);
      lv_obj_center(footer_labels_[index]);
    }
  }

  void refresh_footer(const char *label_a, const char *label_b, const char *label_c) {
    lv_label_set_text(footer_labels_[0], label_a != nullptr ? label_a : "");
    lv_label_set_text(footer_labels_[1], label_b != nullptr ? label_b : "");
    lv_label_set_text(footer_labels_[2], label_c != nullptr ? label_c : "");
  }

  void sync_time() {
    const uint32_t now_ms = millis_now();
    if (have_tick_) {
      lv_tick_inc(now_ms - last_tick_ms_);
    }
    last_tick_ms_ = now_ms;
    have_tick_ = true;
  }

  NativeLcd lcd_{};
  lv_disp_draw_buf_t draw_buf_{};
  lv_disp_drv_t disp_drv_{};
  lv_disp_t *disp_ = nullptr;
  std::array<std::array<char, k_max_line_length>, 4> lines_{};
  std::array<lv_obj_t *, 4> labels_ = {nullptr, nullptr, nullptr, nullptr};
  size_t line_count_ = 0;
  uint32_t last_tick_ms_ = 0;
  bool have_tick_ = false;
  bool configured_ = false;
  bool ui_dirty_ = false;
  bool has_ui_snapshot_ = false;
  ScreenStyle screen_style_ = ScreenStyle::kRegular;
  UiSnapshot last_ui_snapshot_{};
  lv_obj_t *home_title_bar_ = nullptr;
  lv_obj_t *home_title_label_ = nullptr;
  std::array<lv_obj_t *, 4> home_info_labels_ = {nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *home_bar_bg_ = nullptr;
  lv_obj_t *home_bar_fill_ = nullptr;
  lv_obj_t *home_remain_label_ = nullptr;
  lv_obj_t *diagnostics_title_bar_ = nullptr;
  lv_obj_t *diagnostics_title_label_ = nullptr;
  std::array<lv_obj_t *, 4> diagnostics_labels_ = {nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *scale_title_bar_ = nullptr;
  lv_obj_t *scale_title_label_ = nullptr;
  std::array<lv_obj_t *, 3> scale_labels_ = {nullptr, nullptr, nullptr};
  lv_obj_t *rfid_title_bar_ = nullptr;
  lv_obj_t *rfid_title_label_ = nullptr;
  std::array<lv_obj_t *, 4> rfid_labels_ = {nullptr, nullptr, nullptr, nullptr};
  std::array<lv_obj_t *, 3> footer_boxes_ = {nullptr, nullptr, nullptr};
  std::array<lv_obj_t *, 3> footer_labels_ = {nullptr, nullptr, nullptr};
  static lv_color_t draw_buffer_[NativeLcd::kWidth * k_draw_buffer_lines];
};

lv_color_t NativeDisplay::draw_buffer_[NativeLcd::kWidth * k_draw_buffer_lines];

NativeDisplay &display_device() {
  static NativeDisplay device;
  return device;
}
}  // namespace

void begin() {
  diagnostics::log_line("LCD: begin");
  if (!display_device().begin()) {
    diagnostics::log_line("LCD: init failed");
    return;
  }

  diagnostics::log_line("LCD: ready");
}

bool is_narrow() {
  return display_device().is_narrow();
}

void show_boot_test() {
  display_device().show_boot_test();
}

void show_text(const char *line1, const char *line2) {
  display_device().show_text(line1, line2);
}

void show_lines(const char *line1, const char *line2, const char *line3, const char *line4) {
  display_device().show_lines(line1, line2, line3, line4);
}

void show_diagnostics(const char *line1, const char *line2, const char *line3, const char *line4) {
  display_device().show_diagnostics(line1, line2, line3, line4);
}

void render_home(const HomeSnapshot &snapshot) {
  display_device().render_home(snapshot);
}

void render(const UiSnapshot &snapshot) {
  display_device().render(snapshot);
}

void append_line(const char *line) {
  display_device().append_line(line);
}

void show_card_removed() {
  show_text("Card removed", "Waiting for card");
}

void show_uid_and_type(const uint8_t *uid, uint8_t uid_length) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char type_line[48] = {};
  std::snprintf(type_line, sizeof(type_line), "Type: %s", diagnostics::classify_tag_type(uid_length));

  show_text(uid_line, type_line);
}

void show_uid_and_usage(const uint8_t *uid, uint8_t uid_length, uint32_t usage_seconds,
                        uint8_t life_percent) {
  char uid_line[48] = {};
  diagnostics::format_uid(uid, uid_length, uid_line, sizeof(uid_line));

  char usage_line[48] = {};
  std::snprintf(usage_line, sizeof(usage_line), "Usage: %lu s",
                static_cast<unsigned long>(usage_seconds));
  size_t used = std::strlen(usage_line);
  if (used + 1 < sizeof(usage_line)) {
    std::snprintf(usage_line + used, sizeof(usage_line) - used, " Life: %u%%",
                  static_cast<unsigned>(life_percent));
  }

  show_text(uid_line, usage_line);
}

void show_page_read_failed() {
  append_line("Page 24: read failed");
}
}  // namespace display
