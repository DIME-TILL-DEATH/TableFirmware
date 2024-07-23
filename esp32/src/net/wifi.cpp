#include "wifi.hpp"

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"

#include <netdb.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "filemanager/settings.hpp"

static const char *WIFI_TAG = "wifi task";

#define ESP_WIFI_CHANNEL   1
#define MAX_STA_CONN       4

esp_netif_t* netif_ptr;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station "MACSTR" join, AID=%d",MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station "MACSTR" leave, AID=%d",MAC2STR(event->mac), event->aid);
    }
}

void WIFI_Init(void)
{
      //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    netif_ptr = esp_netif_create_default_wifi_ap();

    //esp_netif_dns_info_t DNS_ipInfo{0};
    esp_netif_ip_info_t NETIF_ipInfo;

    esp_netif_dhcps_stop(netif_ptr);
    esp_netif_get_ip_info(netif_ptr, &NETIF_ipInfo);

    NETIF_ipInfo.ip.addr = ipaddr_addr("192.168.1.1");
    NETIF_ipInfo.gw.addr = ipaddr_addr("192.168.1.1");
    NETIF_ipInfo.netmask.addr = ipaddr_addr("255.255.255.0");   
    esp_netif_set_ip_info(netif_ptr, &NETIF_ipInfo);

    //DNS_ipInfo.ip.u_addr = ipaddr_addr("0.0.0.0");
   // IP_ADDR4(&DNS_ipInfo.ip, 0, 0, 0, 0);
    //dhcps_offer_t opt_val = OFFER_DNS;
    //esp_netif_dhcps_option(netif_ptr, TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_DOMAIN_NAME_SERVER, &opt_val, 1);

    esp_netif_dhcps_start(netif_ptr);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    std::string wifiSSID = Settings::getSetting(Settings::String::WIFI_SSID);
    //std::string wifiPassword = Settings::getSetting(Settings::String::WIFI_PASSWORD);
    // std::string wifiSSID = "Kinetic_table";
    std::string wifiPassword = "";

    wifi_config_t wifi_config;
    
    //wifi_config.ap.ssid = (uint8_t*)defaultWifiSSID.c_str();
    memcpy(wifi_config.ap.ssid, wifiSSID.c_str(), wifiSSID.size());
    wifi_config.ap.ssid_len = wifiSSID.size();
    wifi_config.ap.channel = ESP_WIFI_CHANNEL;
    memcpy(wifi_config.ap.password, wifiPassword.c_str(), wifiPassword.size());
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (wifiPassword.size() < 8) 
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_config_80211_tx_rate(WIFI_IF_AP, WIFI_PHY_RATE_MAX);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             wifiSSID.c_str(), wifiPassword.c_str(), ESP_WIFI_CHANNEL);
}