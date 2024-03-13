#ifndef LEDSTRIP_HPP
#define LEDSTRIP_HPP

#include "driver/mcpwm_prelude.h"

class LedStrip
{
public:

    LedStrip();

    void setBrightness(float newBrightness);
    float getBrightness();

private:
    float brightness{0.5};

    mcpwm_timer_handle_t timer{NULL};
    mcpwm_oper_handle_t oper{NULL};
    mcpwm_cmpr_handle_t comparator{NULL};
    mcpwm_gen_handle_t generator{NULL};

    constexpr static char TAG[] = "LED STRIP";

    constexpr static uint32_t period{1000};
};

#endif