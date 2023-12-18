#ifndef USER_WIFI_H
#define USER_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#define EXAMPLE_ESP_WIFI_SSID      "Kinetic_Table"
#define EXAMPLE_ESP_WIFI_PASS      "1234567890"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

void WIFI_Init(void);

#ifdef __cplusplus
}
#endif

#endif