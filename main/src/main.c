#include <math.h>
#include <stdio.h>

#include "common.h"
#include "ir.h"
#include "lcd.h"
#include "render.h"
#include "timer.h"
#include "ui.h"

void app_main(void) {
  lv_init();
  gptimer_handle_t timer = timer_init();
  ssd1306_lcd_panel_t* lcd = lcd_init();
  lv_ui_init(lcd);
  rmt_data_t* rmt_data = rmt_init();

  xTaskCreate(lv_timer_handler_task, LV_TIMER_HANDLER_TASK_NAME,
              LV_TIMER_HANDLER_TASK_STACK_SIZE, lcd,
              LV_TIMER_HANDLER_TASK_PRIORITY, NULL);

  vTaskDelete(NULL);
}
