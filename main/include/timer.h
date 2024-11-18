#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

gptimer_handle_t timer_init(void);
void lv_timer_handler_task(__unused void *pvParameters);

#endif