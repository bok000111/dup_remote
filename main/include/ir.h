#ifndef __IR_H__
#define __IR_H__

#include "common.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "freertos/task.h"
#include "lcd.h"

#define RMT_RESOLUTION_HZ 1000000
#define RMT_TX_GPIO GPIO_NUM_20
#define RMT_RX_GPIO GPIO_NUM_10
#define RMT_MEM_BLOCK_SIZE 64
#define RMT_USE_DMA false
#define RMT_SIG_BITS 12

typedef enum {
  RMT_SIGNAL_UNKNOWN = 0,
  RMT_SIGNAL_FRAME = (uint16_t)(0b11011 << 7),
  RMT_SIGNAL_OFF = (RMT_SIGNAL_FRAME | (1 << 0)),
  RMT_SIGNAL_ON_SPEED = (RMT_SIGNAL_FRAME |
                         (1 << 1)),  // turn on and change speed use same signal
  RMT_SIGNAL_MODE = (RMT_SIGNAL_FRAME | (1 << 2)),
  RMT_SIGNAL_TIMER = (RMT_SIGNAL_FRAME | (1 << 3)),
  RMT_SIGNAL_ROTATE = (RMT_SIGNAL_FRAME | (1 << 4)),
} rmt_signal_t;

typedef struct {
  rmt_channel_handle_t tx_channel;
  rmt_channel_handle_t rx_channel;
  QueueHandle_t rx_queue;
  rmt_encoder_handle_t tx_encoder;
} rmt_data_t;

rmt_data_t *rmt_init(void);
void rmt_tx_setup(rmt_data_t *rmt_data);
void rmt_rx_setup(rmt_data_t *rmt_data);
void rmt_task(void *pvParameters);

#endif  // __IR_H__