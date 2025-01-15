#ifndef WLEDSTRIP_HPP
#define WLEDSTRIP_HPP

#include "abstractledstrip.hpp"

#include "led_strip.h"

class WLedStrip : public AbstractLedStrip
{
public:
    typedef struct
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    }Color;
    

    WLedStrip(uint16_t ledCount);

    
    void updateFrame();
    void refresh();

private:
    led_strip_handle_t ledStripHandle;

    uint16_t m_ledCount;

    Color* ledColors;

    Color hueToRgb(uint16_t hue, uint8_t saturation, uint8_t value);

    int32_t currentFrame=0;
};

#endif