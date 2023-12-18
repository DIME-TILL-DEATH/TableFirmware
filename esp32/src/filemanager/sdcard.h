#ifndef SDCARD_H
#define SDCARD_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOUNT_POINT "/sdcard"

esp_err_t SDBUS_Init(void);
esp_err_t SDCARD_Mount(void);

#ifdef __cplusplus
}
#endif

#endif 