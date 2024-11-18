#include "ir.h"

#include "lcd.h"

static size_t rmx_tx_encoder(const void *data, size_t data_size,
                             size_t symbols_written, size_t symbols_free,
                             rmt_symbol_word_t *symbols, bool *done, void *arg);
static rmt_signal_t rmt_rx_parse_symbol(rmt_rx_done_event_data_t *edata);
static bool rmt_rx_done_cb(rmt_channel_handle_t channel,
                           const rmt_rx_done_event_data_t *edata,
                           void *user_data);
static char *signal_to_str(rmt_signal_t signal);

rmt_data_t *rmt_init(void) {
  rmt_data_t *rmt_data =
      heap_caps_calloc(1, sizeof(rmt_data_t), MALLOC_CAP_8BIT);
  CHECK_ALLOC(rmt_data);
  // rmt_tx_setup(rmt_data);
  rmt_rx_setup(rmt_data);

  xTaskCreate(rmt_task, RMT_TASK_NAME, RMT_TASK_STACK_SIZE, rmt_data,
              RMT_TASK_PRIORITY, NULL);
  return rmt_data;
}

void rmt_tx_setup(rmt_data_t *rmt_data) {
  rmt_tx_channel_config_t tx_conf = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .gpio_num = RMT_TX_GPIO,
      .mem_block_symbols = RMT_MEM_BLOCK_SIZE,
      .resolution_hz = RMT_RESOLUTION_HZ,
      .trans_queue_depth = 4,
      .flags.invert_out = false,
      .flags.with_dma = RMT_USE_DMA,
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_conf, &rmt_data->tx_channel));
  ESP_LOGD(LOG_TAG, "RMT TX channel initialized");

  rmt_simple_encoder_config_t encoder_conf = {
      .callback = rmx_tx_encoder,
      .arg = NULL,
      .min_chunk_size = 12,
  };
  rmt_new_simple_encoder(&encoder_conf, &rmt_data->tx_encoder);

  ESP_ERROR_CHECK(rmt_enable(rmt_data->tx_channel));
}

void rmt_rx_setup(rmt_data_t *rmt_data) {
  rmt_rx_channel_config_t rx_conf = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .gpio_num = RMT_RX_GPIO,
      .mem_block_symbols = RMT_MEM_BLOCK_SIZE,
      .resolution_hz = RMT_RESOLUTION_HZ,
      .flags.invert_in = false,
      .flags.with_dma = RMT_USE_DMA,
  };
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_conf, &rmt_data->rx_channel));
  ESP_LOGD(LOG_TAG, "RMT RX channel initialized");

  rmt_data->rx_queue = xQueueCreate(8, sizeof(rmt_rx_done_event_data_t));
  CHECK_ALLOC(rmt_data->rx_queue);
  rmt_rx_event_callbacks_t rx_cbs = {
      .on_recv_done = rmt_rx_done_cb,
  };
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rmt_data->rx_channel, &rx_cbs,
                                                  rmt_data->rx_queue));
  ESP_LOGD(LOG_TAG, "RMT RX callbacks registered");

  ESP_ERROR_CHECK(rmt_enable(rmt_data->rx_channel));
}

void rmt_task(void *pvParameters) {
  rmt_symbol_word_t *rx_symbols = heap_caps_calloc(
      RMT_SIG_BITS * 3, sizeof(rmt_symbol_word_t), MALLOC_CAP_8BIT);
  CHECK_ALLOC(rx_symbols);

  const rmt_receive_config_t rx_cfg = {
      .signal_range_min_ns = 1000,              // 1us
      .signal_range_max_ns = 16 * 1000 * 1000,  // 16ms
  };
  // const rmt_transmit_config_t tx_cfg = {
  //     .loop_count = 0,
  //     .flags.eot_level = 1,
  //     .flags.queue_nonblocking = false,
  // };

  rmt_data_t *rmt_data = pvParameters;
  lv_obj_t *scr = lv_scr_act();
  ssd1306_lcd_panel_t *lcd = lv_obj_get_user_data(scr);
  lv_obj_t *rmt_label = lv_obj_get_child(lv_obj_get_child(scr, 0), 1);

  do {
    rmt_rx_done_event_data_t rx_data;

    rmt_receive(rmt_data->rx_channel, rx_symbols,
                sizeof(rmt_symbol_word_t) * RMT_SIG_BITS * 3, &rx_cfg);
    xQueueReceive(rmt_data->rx_queue, &rx_data, portMAX_DELAY);

    rmt_signal_t signal = rmt_rx_parse_symbol(&rx_data);
    char *signal_str = NULL;
    switch (signal) {
      case RMT_SIGNAL_OFF:
        signal_str = "RX:OFF";
        break;
      case RMT_SIGNAL_ON_SPEED:
        signal_str = "RX:ON/SPEED";
        break;
      case RMT_SIGNAL_MODE:
        signal_str = "RX:MODE";
        break;
      case RMT_SIGNAL_TIMER:
        signal_str = "RX:TIMER";
        break;
      case RMT_SIGNAL_ROTATE:
        signal_str = "RX:ROTATE";
        break;
      default:
        signal_str = "RX:UNKNOWN";
    };

    if (xSemaphoreTake(lcd->lvgl_mutex,
                       pdMS_TO_TICKS(LV_UI_REFRESH_PERIOD_MS)) == pdTRUE) {
      lv_label_set_text(rmt_label, signal_str);
      xSemaphoreGive(lcd->lvgl_mutex);
    } else {
      ESP_LOGW(LOG_TAG, "RMT RX: failed to update label");
    }

  } while (true);
}

// does not work for now
static size_t rmx_tx_encoder(const void *data, size_t data_size,
                             size_t symbols_written, size_t symbols_free,
                             rmt_symbol_word_t *symbols, bool *done,
                             void *arg) {
  const rmt_symbol_word_t signal0 = {
      .duration0 = 400,
      .level0 = 0,
      .duration1 = 1300,
      .level1 = 1,
  };
  const rmt_symbol_word_t signal1 = {
      .duration0 = 1300,
      .level0 = 0,
      .duration1 = 400,
      .level1 = 1,
  };
  const bool *signal_bits = data;

  if (symbols_free < RMT_SIG_BITS - symbols_written) {
    return 0;
  }

  for (int i = 0; i < RMT_SIG_BITS; i++) {
    symbols[i] = signal_bits[i] ? signal1 : signal0;
  }

  *done = true;
  return RMT_SIG_BITS - symbols_written;
}

static rmt_signal_t rmt_rx_parse_symbol(rmt_rx_done_event_data_t *edata) {
  uint16_t signal = 0;

  if (edata->num_symbols < RMT_SIG_BITS) {
    ESP_LOGW(LOG_TAG, "RMT RX: invalid signal length: %d", edata->num_symbols);
    return RMT_SIGNAL_UNKNOWN;
  }

  for (int i = 0; i < RMT_SIG_BITS; i++) {
    ESP_LOGD(
        LOG_TAG, "%d:[%d:%d]:[%d:%d]", i, edata->received_symbols[i].level0,
        edata->received_symbols[i].duration0, edata->received_symbols[i].level1,
        edata->received_symbols[i].duration1);
    uint16_t d0 = edata->received_symbols[i].duration0;

    if (1100 < d0 && d0 < 1500) {
      signal = (signal << 1) | 1;
    } else {
      signal = (signal << 1) | 0;
    }
  }

  switch (signal) {
    case RMT_SIGNAL_OFF:
      return RMT_SIGNAL_OFF;
    case RMT_SIGNAL_ON_SPEED:
      return RMT_SIGNAL_ON_SPEED;
    case RMT_SIGNAL_MODE:
      return RMT_SIGNAL_MODE;
    case RMT_SIGNAL_TIMER:
      return RMT_SIGNAL_TIMER;
    case RMT_SIGNAL_ROTATE:
      return RMT_SIGNAL_ROTATE;
    default:
      ESP_LOGW(LOG_TAG, "RMT RX: unknown signal: %d", signal);
      return RMT_SIGNAL_UNKNOWN;
  };
}

static bool IRAM_ATTR rmt_rx_done_cb(rmt_channel_handle_t channel,
                                     const rmt_rx_done_event_data_t *edata,
                                     void *user_data) {
  BaseType_t high_task_wakeup = pdFALSE;
  QueueHandle_t rx_queue = (QueueHandle_t)user_data;
  xQueueSendFromISR(rx_queue, edata, &high_task_wakeup);
  return high_task_wakeup == pdTRUE;
}

static char *signal_to_str(rmt_signal_t signal) {
  char *str =
      heap_caps_malloc((RMT_SIG_BITS + 1) * sizeof(char), MALLOC_CAP_8BIT);
  CHECK_ALLOC(str);
  for (int i = 0; i < RMT_SIG_BITS; i++) {
    str[i] = signal & (1 << (RMT_SIG_BITS - i - 1)) ? '1' : '0';
  }
  str[RMT_SIG_BITS] = '\0';
  return str;
}