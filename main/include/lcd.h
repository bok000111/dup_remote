#ifndef __LCD_H__
#define __LCD_H__

#include "common.h"

typedef struct ssd1306_lcd_panel_s {
  i2c_master_bus_handle_t i2c_bus;
  esp_lcd_panel_io_handle_t io_handle;
  esp_lcd_panel_handle_t panel_handle;
  lv_display_t *lv_disp;
  uint8_t *lcd_buf;    // do not directly access this buffer except in flush_cb
  uint8_t *draw_buf0;  // do not directly access this buffer
  uint8_t *draw_buf1;  // do not directly access this buffer
  void *canvas_buf;    // do not directly access this buffer
  SemaphoreHandle_t lcd_buf_mutex;
  SemaphoreHandle_t lvgl_mutex;
} ssd1306_lcd_panel_t;

ssd1306_lcd_panel_t *lcd_init(void);

#endif  // __LCD_H__