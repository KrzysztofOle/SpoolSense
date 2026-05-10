/**
 * Native LVGL UI layer for the sandbox app.
 *
 * Features (EN):
 * - Configures LVGL display buffers and flush callback for NativeLcd.
 * - Shows an LCD color test with all RGB combinations as large swatches.
 * - Keeps frame updates lightweight while staying fully native ESP-IDF.
 *
 * Funkcje (PL):
 * - Konfiguruje bufory LVGL i flush callback dla NativeLcd.
 * - Pokazuje plansze kolorow z duzymi kwadratami do oceny LCD.
 * - Utrzymuje lekkie aktualizacje klatek bez Arduino.
 *
 * File: sandbox/espidf_native/main/src/native_lvgl_ui.cpp
 */

#include "sandbox/native_lvgl_ui.hpp"

#include <array>
#include <cstdio>

#include "esp_log.h"
#include "lvgl.h"

namespace sandbox {
namespace {
constexpr int k_draw_buffer_lines = 30;
static lv_color_t s_draw_buffer[NativeLcd::kWidth * k_draw_buffer_lines];
static uint16_t s_flush_color_buffer[NativeLcd::kWidth * k_draw_buffer_lines];
static lv_disp_draw_buf_t s_draw_ctx;
static lv_disp_drv_t s_disp_drv;

struct ColorSwatch {
  const char *label;
  lv_color_t color;
  lv_color_t text_color;
};

constexpr lv_coord_t k_title_y = 8;
constexpr lv_coord_t k_subtitle_y = 34;
constexpr lv_coord_t k_grid_top = 68;
constexpr lv_coord_t k_grid_left = 15;
constexpr lv_coord_t k_swatch_w = 68;
constexpr lv_coord_t k_swatch_h = 56;
constexpr lv_coord_t k_gap_x = 6;
constexpr lv_coord_t k_gap_y = 10;
constexpr lv_coord_t k_footer_y = -10;

constexpr int k_swatch_count = 8;

const char *screen_title() {
  return "RGB COMBINATIONS";
}

const char *screen_subtitle() {
  return "A/C COUNTS  B RGB/BGR";
}

std::array<ColorSwatch, k_swatch_count> make_swatches() {
  return {{
      {"000", lv_color_hex(0x000000), lv_color_hex(0xFFFFFF)},
      {"001", lv_color_hex(0x0000FF), lv_color_hex(0xFFFFFF)},
      {"010", lv_color_hex(0x00FF00), lv_color_hex(0x000000)},
      {"011", lv_color_hex(0x00FFFF), lv_color_hex(0x000000)},
      {"100", lv_color_hex(0xFF0000), lv_color_hex(0xFFFFFF)},
      {"101", lv_color_hex(0xFF00FF), lv_color_hex(0xFFFFFF)},
      {"110", lv_color_hex(0xFFFF00), lv_color_hex(0x000000)},
      {"111", lv_color_hex(0xFFFFFF), lv_color_hex(0x000000)},
  }};
}

void create_swatch(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                   const ColorSwatch &swatch) {
  auto *box = lv_obj_create(parent);
  lv_obj_remove_style_all(box);
  lv_obj_set_pos(box, x, y);
  lv_obj_set_size(box, w, h);
  lv_obj_set_style_bg_color(box, swatch.color, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(box, 6, LV_PART_MAIN);
  lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
  lv_obj_set_style_border_color(box, lv_color_hex(0x202020), LV_PART_MAIN);
  lv_obj_set_style_pad_all(box, 4, LV_PART_MAIN);

  auto *label = lv_label_create(box);
  lv_obj_set_style_text_color(label, swatch.text_color, LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_label_set_text(label, swatch.label);
  lv_obj_center(label);
}

void clear_screen(lv_obj_t *scr) {
  lv_obj_clean(scr);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
}

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
  for (int i = 0; i < px_count; ++i) {
    s_flush_color_buffer[i] = color_p[i].full;
  }

  if (lcd->send_area(x1, y1, x2, y2, s_flush_color_buffer) != ESP_OK) {
    ESP_LOGE("sandbox", "LVGL flush send_area failed");
  } else if (!lcd->wait_tx_done()) {
    ESP_LOGE("sandbox", "LVGL flush timeout");
  }

  lv_disp_flush_ready(disp_drv);
}
}  // namespace

bool NativeLvglUi::begin(NativeLcd *lcd) {
  if (lcd == nullptr) {
    return false;
  }
  lcd_ = lcd;

  lv_init();

  lv_disp_draw_buf_init(&s_draw_ctx, s_draw_buffer, nullptr, NativeLcd::kWidth * k_draw_buffer_lines);

  lv_disp_drv_init(&s_disp_drv);
  s_disp_drv.hor_res = NativeLcd::kWidth;
  s_disp_drv.ver_res = NativeLcd::kHeight;
  s_disp_drv.flush_cb = flush_cb;
  s_disp_drv.draw_buf = &s_draw_ctx;
  s_disp_drv.user_data = lcd_;
  auto *disp = lv_disp_drv_register(&s_disp_drv);
  if (disp == nullptr) {
    return false;
  }
  disp_ = disp;

  if (!set_color_order(color_order_)) {
    return false;
  }

  rebuild_screen();
  return true;
}

bool NativeLvglUi::toggle_color_order() {
  return set_color_order(next_color_order_of(color_order_));
}

bool NativeLvglUi::set_color_order(ColorOrder order) {
  if (lcd_ == nullptr) {
    return false;
  }

  color_order_ = order;
  if (lcd_->set_color_order(order) != ESP_OK) {
    ESP_LOGE("sandbox", "LCD color order update failed");
    return false;
  }

  return true;
}

void NativeLvglUi::update(const ButtonSnapshot &buttons, const ButtonCounters &counters, uint32_t uptime_ms) {
  if (footer_ == nullptr) {
    return;
  }

  char line_footer[128];
  std::snprintf(line_footer, sizeof(line_footer), "%s | %s | A:%d B:%d C:%d | UP %lus | CNT %lu %lu %lu",
                screen_title(), color_order_name(color_order_), static_cast<int>(buttons.a),
                static_cast<int>(buttons.b), static_cast<int>(buttons.c), static_cast<unsigned long>(uptime_ms / 1000U),
                static_cast<unsigned long>(counters.a), static_cast<unsigned long>(counters.b),
                static_cast<unsigned long>(counters.c));
  lv_label_set_text(static_cast<lv_obj_t *>(footer_), line_footer);
}

void NativeLvglUi::tick(uint32_t elapsed_ms) {
  lv_tick_inc(elapsed_ms);
}

void NativeLvglUi::process() {
  lv_timer_handler();
}

void NativeLvglUi::rebuild_screen() {
  auto *scr = lv_scr_act();
  clear_screen(scr);

  auto *title = lv_label_create(scr);
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_label_set_text(title, screen_title());
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, k_title_y);
  title_ = title;

  auto *subtitle = lv_label_create(scr);
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0xA0A0A0), LV_PART_MAIN);
  lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_label_set_text(subtitle, screen_subtitle());
  lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, k_subtitle_y);
  subtitle_ = subtitle;

  const auto swatches = make_swatches();
  for (int index = 0; index < k_swatch_count; ++index) {
    const int column = index % 4;
    const int row = index / 4;
    const lv_coord_t x = k_grid_left + static_cast<lv_coord_t>(column) * (k_swatch_w + k_gap_x);
    const lv_coord_t y = k_grid_top + static_cast<lv_coord_t>(row) * (k_swatch_h + k_gap_y);
    create_swatch(scr, x, y, k_swatch_w, k_swatch_h, swatches[index]);
  }

  auto *footer = lv_label_create(scr);
  lv_obj_set_style_text_color(footer, lv_color_hex(0xC0C0C0), LV_PART_MAIN);
  lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_label_set_text(footer, "");
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, k_footer_y);
  footer_ = footer;

  lv_refr_now(nullptr);
}

const char *NativeLvglUi::color_order_name(ColorOrder order) {
  switch (order) {
    case ColorOrder::kRgb:
      return "RGB";
    case ColorOrder::kBgr:
      return "BGR";
  }

  return "UNK";
}

ColorOrder NativeLvglUi::next_color_order_of(ColorOrder order) {
  switch (order) {
    case ColorOrder::kRgb:
      return ColorOrder::kBgr;
    case ColorOrder::kBgr:
      return ColorOrder::kRgb;
  }

  return ColorOrder::kRgb;
}
}  // namespace sandbox
