#include "ui.h"

static void update_stat_label_cb(lv_timer_t *timer);

void lv_ui_init(ssd1306_lcd_panel_t *lcd) {
  lv_obj_t *scr = lv_display_get_screen_active(lcd->lv_disp);
  lv_obj_set_user_data(scr, lcd);

  // global flex container
  lv_obj_t *cont_col = lv_obj_create(scr);
  lv_obj_set_size(cont_col, LCD_WIDTH, LCD_HEIGHT);
  lv_obj_align(cont_col, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_START);
  lv_obj_set_style_radius(cont_col, 0, 0);
  lv_obj_set_style_border_width(cont_col, 0, 0);
  lv_obj_set_style_pad_all(cont_col, 2, 0);
  lv_obj_set_style_pad_gap(cont_col, 2, 0);
  lv_obj_set_style_bg_color(cont_col, lv_color_black(), 0);

  // status label
  lv_obj_t *stat = lv_label_create(cont_col);
  lv_obj_set_user_data(stat, lcd);
  lv_obj_set_size(stat, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_align(stat, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_text_color(stat, lv_color_white(), 0);
  lv_label_set_text(stat, "M:-% T:-C");
  lv_timer_create(update_stat_label_cb, 1000, stat);

  // rmt label
  lv_obj_t *rmt = lv_label_create(cont_col);
  lv_obj_set_user_data(rmt, lcd);
  lv_obj_set_size(rmt, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_align(rmt, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_text_color(rmt, lv_color_white(), 0);
  lv_label_set_text(rmt, "RX:-");
}

static void update_stat_label_cb(lv_timer_t *timer) {
  static temperature_sensor_handle_t temp_handle = NULL;

  if (temp_handle == NULL) {
    temperature_sensor_config_t temp_config = {
        .range_min = 0.0f,
        .range_max = 80.0f,
    };
    temperature_sensor_install(&temp_config, &temp_handle);
    temperature_sensor_enable(temp_handle);
  }

  lv_obj_t *label = lv_timer_get_user_data(timer);
  ssd1306_lcd_panel_t *lcd = lv_obj_get_user_data(label);

  const size_t total_mem = heap_caps_get_total_size(MALLOC_CAP_8BIT);
  const size_t free_mem = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  float temperature = 0.0f;
  ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &temperature));

  if (xSemaphoreTake(lcd->lvgl_mutex, pdMS_TO_TICKS(LV_UI_REFRESH_PERIOD_MS)) ==
      pdTRUE) {
    lv_label_set_text_fmt(label, "M:%d%% T:%.0fC",
                          ((total_mem - free_mem) * 100) / (total_mem),
                          temperature);
    xSemaphoreGive(lcd->lvgl_mutex);
  } else {
    ESP_LOGW(LOG_TAG, "Failed to take LVGL mutex");
  }
}
