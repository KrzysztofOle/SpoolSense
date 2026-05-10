/**
 * Native LVGL UI layer for the sandbox app.
 *
 * Features (EN):
 * - Configures LVGL display buffers and flush callback for NativeLcd.
 * - Shows button states and click counters on a simple dashboard screen.
 * - Keeps frame updates lightweight while staying fully native ESP-IDF.
 *
 * Funkcje (PL):
 * - Konfiguruje bufory LVGL i flush callback dla NativeLcd.
 * - Pokazuje stany przyciskow i liczniki klikniec na prostym ekranie.
 * - Utrzymuje lekkie aktualizacje klatek bez Arduino.
 *
 * File: sandbox/espidf_native/main/src/native_lvgl_ui.cpp
 */

#include "sandbox/native_lvgl_ui.hpp"

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

  auto *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

  auto *title = lv_label_create(scr);
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_label_set_text(title, "sandbox/espidf_native");
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 10);
  title_ = title;

  auto *buttons = lv_label_create(scr);
  lv_obj_set_style_text_color(buttons, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(buttons, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_label_set_text(buttons, "Buttons A:0 B:0 C:0");
  lv_obj_align(buttons, LV_ALIGN_TOP_LEFT, 10, 64);
  buttons_ = buttons;

  auto *counters = lv_label_create(scr);
  lv_obj_set_style_text_color(counters, lv_color_hex(0x00E5FF), LV_PART_MAIN);
  lv_obj_set_style_text_font(counters, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_label_set_text(counters, "Clicks  A:0 B:0 C:0");
  lv_obj_align(counters, LV_ALIGN_TOP_LEFT, 10, 108);
  counters_ = counters;

  auto *uptime = lv_label_create(scr);
  lv_obj_set_style_text_color(uptime, lv_color_hex(0xFFD54F), LV_PART_MAIN);
  lv_obj_set_style_text_font(uptime, &lv_font_montserrat_22, LV_PART_MAIN);
  lv_label_set_text(uptime, "Uptime: 0s");
  lv_obj_align(uptime, LV_ALIGN_TOP_LEFT, 10, 152);
  uptime_ = uptime;

  auto make_rgb_box = [scr](lv_coord_t x, lv_coord_t y, lv_color_t color, const char *text) {
    auto *box = lv_obj_create(scr);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, 64, 28);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_style_bg_color(box, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(box, 4, LV_PART_MAIN);

    auto *label = lv_label_create(box);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_obj_center(label);
  };

  make_rgb_box(10, 204, lv_color_hex(0xD50000), "R");
  make_rgb_box(88, 204, lv_color_hex(0x00A000), "G");
  make_rgb_box(166, 204, lv_color_hex(0x0040FF), "B");

  lv_refr_now(nullptr);
  return true;
}

void NativeLvglUi::update(const ButtonSnapshot &buttons, const ButtonCounters &counters, uint32_t uptime_ms) {
  if (buttons_ == nullptr || counters_ == nullptr || uptime_ == nullptr) {
    return;
  }

  char line_buttons[64];
  std::snprintf(line_buttons, sizeof(line_buttons), "Buttons A:%d B:%d C:%d",
                static_cast<int>(buttons.a), static_cast<int>(buttons.b), static_cast<int>(buttons.c));
  lv_label_set_text(static_cast<lv_obj_t *>(buttons_), line_buttons);

  char line_counters[64];
  std::snprintf(line_counters, sizeof(line_counters), "Clicks  A:%lu B:%lu C:%lu",
                static_cast<unsigned long>(counters.a), static_cast<unsigned long>(counters.b),
                static_cast<unsigned long>(counters.c));
  lv_label_set_text(static_cast<lv_obj_t *>(counters_), line_counters);

  char line_uptime[48];
  std::snprintf(line_uptime, sizeof(line_uptime), "Uptime: %lus", static_cast<unsigned long>(uptime_ms / 1000U));
  lv_label_set_text(static_cast<lv_obj_t *>(uptime_), line_uptime);
}

void NativeLvglUi::tick(uint32_t elapsed_ms) {
  lv_tick_inc(elapsed_ms);
}

void NativeLvglUi::process() {
  lv_timer_handler();
}
}  // namespace sandbox
