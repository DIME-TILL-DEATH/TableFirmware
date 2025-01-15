#ifndef LEDSTASK_HPP
#define LEDTASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

void leds_task(void *arg);

#endif