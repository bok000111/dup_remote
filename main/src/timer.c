
#include "timer.h"

#include "driver/gptimer.h"

static bool IRAM_ATTR lv_tick_inc_timer_cb(
    gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata,
    __unused void *user_ctx) {
  lv_tick_inc(LV_TICK_PERIOD_MS);
  return true;
}

gptimer_handle_t timer_init(void) {
  gptimer_handle_t timer;
  gptimer_config_t timer_config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = 1 * 1000 * 1000,  // 1MHz
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
  gptimer_alarm_config_t alarm_config = {
      .reload_count = 0,
      .alarm_count = LV_TICK_PERIOD_MS * 1000,
      .flags.auto_reload_on_alarm = true,
  };
  ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config));
  gptimer_event_callbacks_t timer_cbs = {
      .on_alarm = lv_tick_inc_timer_cb,
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &timer_cbs, NULL));
  ESP_ERROR_CHECK(gptimer_enable(timer));
  ESP_ERROR_CHECK(gptimer_start(timer));

  return timer;
}

void lv_timer_handler_task(__unused void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    lv_timer_handler();  // LVGL 메인 루프 호출
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(LV_UI_REFRESH_PERIOD_MS));
  }
}