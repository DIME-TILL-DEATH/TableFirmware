#include "intervaltimer.hpp"

IntervalTimer::IntervalTimer(gptimer_alarm_cb_t newIsrCallback)
{
    isrCallback = newIsrCallback;

    gptimer_config_t timer_config;
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us
    timer_config.flags.intr_shared = false;

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timerHandle));

    gptimer_event_callbacks_t timerEventsCallbacks = {
        .on_alarm = isrCallback
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timerHandle, &timerEventsCallbacks, NULL));

    ESP_ERROR_CHECK(gptimer_enable(timerHandle));
    ESP_ERROR_CHECK(gptimer_start(timerHandle));
}

void IntervalTimer::setInterval(uint16_t interval)
{
    gptimer_alarm_config_t alarm_config;
    alarm_config.alarm_count = interval;
    alarm_config.reload_count = 0;
    alarm_config.flags.auto_reload_on_alarm = true;

    ESP_ERROR_CHECK(gptimer_set_alarm_action(timerHandle, &alarm_config));
}

void IntervalTimer::start()
{
    gptimer_enable(timerHandle);
    gptimer_start(timerHandle);
}

void IntervalTimer::stop()
{
    gptimer_stop(timerHandle);
    gptimer_disable(timerHandle);
}