#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "sdcard.h"

static const char *SDCARD_TAG = "SDCARD task";

#define PIN_NUM_MISO  12
#define PIN_NUM_MOSI  13
#define PIN_NUM_CLK   14
#define PIN_NUM_CS    15

esp_err_t SDBUS_Init(void)
{
    esp_err_t ret;

    ESP_LOGI(SDCARD_TAG, "Initializing SD bus");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, 1);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(SDCARD_TAG, "Failed to initialize bus.");
        return ret;
    }
    return ESP_OK;
}

esp_err_t SDCARD_Mount(void)
{
    esp_err_t ret;

    sdmmc_card_t *card;    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdspi_device_config_t device_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    device_config.gpio_cs = PIN_NUM_CS;
    device_config.host_id = host.slot;

    ESP_LOGI(SDCARD_TAG, "Mounting filesystem");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = 
    {
        .format_if_mount_failed = false,
        .max_files = 16,
        .allocation_unit_size = 16 * 1024
    };

    const char mount_point[] = MOUNT_POINT;
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &device_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(SDCARD_TAG, "Failed to mount filesystem.");
        } 
        else 
        {
            ESP_LOGE(SDCARD_TAG, "Failed to initialize the card (%s).", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(SDCARD_TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}