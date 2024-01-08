#ifndef TCPIP_H
#define TCPIP_H

#include "freertos/task.h"
#include "freertos/queue.h"

void TCPIP_Init(void);

extern QueueHandle_t printReqQueue, fileReqQueue, netAnswQueue;

#endif