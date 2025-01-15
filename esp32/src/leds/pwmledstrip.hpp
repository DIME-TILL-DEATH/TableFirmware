#ifndef PWMLEDSTRIP_HPP
#define PWMLEDSTRIP_HPP

#include "abstractledstrip.hpp"

#include "driver/mcpwm_prelude.h"

class PwmLedStrip : public AbstractLedStrip
{
public:

    PwmLedStrip();

    void setBrightness(float newBrightness) override;

private:

    mcpwm_timer_handle_t timer{NULL};
    mcpwm_oper_handle_t oper{NULL};
    mcpwm_cmpr_handle_t comparator{NULL};
    mcpwm_gen_handle_t generator{NULL};

    constexpr static uint32_t period{1000};
};

#endif