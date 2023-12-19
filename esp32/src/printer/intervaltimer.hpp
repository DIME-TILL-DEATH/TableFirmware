#ifndef INTERVALTIMER_H
#define INTERVALTIMER_H

#include "driver/gptimer.h"

class IntervalTimer
{
    public:
        explicit IntervalTimer(gptimer_alarm_cb_t newIsrCallback);
        IntervalTimer() = delete;

        void setInterval(uint16_t interval);
        void start();
        void stop();
    private:
        gptimer_handle_t timerHandle{NULL};
        gptimer_alarm_cb_t isrCallback{NULL};
};

#endif