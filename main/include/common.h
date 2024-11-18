#ifndef __COMMON_H__
#define __COMMON_H__

#include "driver/gptimer_types.h"
#include "driver/i2c_types.h"
#include "driver/rmt_types.h"
#include "driver/temperature_sensor.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#define LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define LCD_PIN_NUM_SDA GPIO_NUM_8
#define LCD_PIN_NUM_SCL GPIO_NUM_9
#define LCD_CONTROLLER "SSD1306"
#define LCD_I2C_HW_ADDR 0x3C
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LCD_BUF_SIZE (LCD_WIDTH * LCD_HEIGHT / 8)

#define LV_PALETTE_SIZE 8
#define LVGL_DRAW_BUF_SIZE (LCD_BUF_SIZE + LV_PALETTE_SIZE)

#define LV_TICK_PERIOD_MS 8
#define LV_TIMER_HANDLER_TASK_NAME "lv_timer_task"
#define LV_TIMER_HANDLER_TASK_STACK_SIZE 8192
#define LV_TIMER_HANDLER_TASK_PRIORITY 3
#define LV_UI_REFRESH_PERIOD_MS 16

#define RMT_TASK_NAME "rmt_task"
#define RMT_TASK_STACK_SIZE 8192
#define RMT_TASK_PRIORITY 5

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOG_TAG __FILE__ ":" TOSTRING(__LINE__)
#define CHECK_ALLOC(ptr)                                        \
  do {                                                          \
    if (ptr == NULL) {                                          \
      ESP_LOGE(LOG_TAG, "Failed to allocate memory: %s", #ptr); \
      abort();                                                  \
    }                                                           \
  } while (0)

#endif  // __COMMON_H__