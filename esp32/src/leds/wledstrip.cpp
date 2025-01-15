#include "wledstrip.hpp"

#include <math.h>

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"

WLedStrip::WLedStrip(uint16_t ledCount)
{
    m_ledCount = ledCount;
    ledColors = new Color[ledCount];

    for (int i = 0; i < m_ledCount; i++) 
    {
        ledColors[i].red = 0;
        ledColors[i].green = 0;
        ledColors[i].blue = 0;
    }

    led_strip_config_t strip_config;
    strip_config.strip_gpio_num = 26;//23;   
    strip_config.max_leds = m_ledCount;        
    strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB; 
    strip_config.led_model = LED_MODEL_WS2815;           
    strip_config.flags.invert_out = false;               

    // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
    uint32_t LED_STRIP_RMT_RES_HZ =  (10 * 1000 * 1000);

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config;
    rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;        // different clock source can lead to different power consumption
    rmt_config.resolution_hz = LED_STRIP_RMT_RES_HZ; // RMT counter clock frequency
    rmt_config.flags.with_dma = false;               

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &ledStripHandle));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
}

#define frames 100
int dir = 1;
void WLedStrip::updateFrame()
{
    for (int i = 0; i < m_ledCount; i++) 
    {
            ledColors[i].red = 15+pow(M_E, (float)currentFrame/18.5);
            ledColors[i].green = 0;
            ledColors[i].blue = 0;    
    }
    //ESP_LOGI("LED", "%d : %d", cur, ledColors[0].red);

    if(currentFrame == frames) 
    {
        dir = -1; //50 frames per second, 5 seconds
        //ESP_LOGI("LED", "dir: %d", dir);
    }

    if(currentFrame == 0)
    {
        dir = 1;
    }
    
    currentFrame+=dir;

    refresh();
}

void WLedStrip::refresh()
{
    for (int i = 0; i < m_ledCount; i++) 
    {
        led_strip_set_pixel(ledStripHandle, i, ledColors[i].green, ledColors[i].red, ledColors[i].blue);     
    }
    led_strip_refresh(ledStripHandle);

    //led_strip_clear(led_strip);    
}

WLedStrip::Color WLedStrip::hueToRgb(uint16_t hue, uint8_t saturation, uint8_t value)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;

    uint32_t rgb_max = value;
    uint32_t rgb_min = rgb_max * (255 - saturation) / 255.0f;

    uint32_t i = hue / 60;
    uint32_t diff = hue % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        red = rgb_max;
        green = rgb_min + rgb_adj;
        blue = rgb_min;
        break;
    case 1:
        red = rgb_max - rgb_adj;
        green = rgb_max;
        blue = rgb_min;
        break;
    case 2:
        red = rgb_min;
        green = rgb_max;
        blue = rgb_min + rgb_adj;
        break;
    case 3:
        red = rgb_min;
        green = rgb_max - rgb_adj;
        blue = rgb_max;
        break;
    case 4:
        red = rgb_min + rgb_adj;
        green = rgb_min;
        blue = rgb_max;
        break;
    default:
        red = rgb_max;
        green = rgb_min;
        blue = rgb_max - rgb_adj;
        break;
    }

    Color retVal;

    retVal.red = red;
    retVal.green = green;
    retVal.blue = blue;

    return retVal;
}