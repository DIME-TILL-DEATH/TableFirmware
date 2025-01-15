#include "ledstask.hpp"

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"


#include "wledstrip.hpp"


void leds_task(void *arg)
{
    WLedStrip WLedStrip(100);

    for(;;)
    {    
        WLedStrip.updateFrame();
        vTaskDelay(pdMS_TO_TICKS(40));   
    }
}