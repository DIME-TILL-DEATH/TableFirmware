#include "pwmledstrip.hpp"

#include "filemanager/settings.hpp"

PwmLedStrip::PwmLedStrip()
{
    // mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config;
    timer_config.group_id = 0;
    timer_config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
    timer_config.resolution_hz = 1000000;  // 1MHz, 1us per tick
    timer_config.period_ticks = period;    // 20000 ticks, 20ms
    timer_config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
    timer_config.flags.update_period_on_empty = false;
    timer_config.flags.update_period_on_sync = false;
    mcpwm_new_timer(&timer_config, &timer);

    // mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config;
    operator_config.group_id = 0; // operator must be in the same group to the timer
    mcpwm_new_operator(&operator_config, &oper);
    mcpwm_operator_connect_timer(oper, timer);

    // mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config;
    comparator_config.flags.update_cmp_on_tez = true;
    mcpwm_new_comparator(oper, &comparator_config, &comparator);

    // mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config;
    generator_config.gen_gpio_num = 32;
    mcpwm_new_generator(oper, &generator_config, &generator);

    // set the initial compare value
    brightness = Settings::getSetting(Settings::Digit::LED_BRIGHTNESS);
    // mcpwm_comparator_set_compare_value(comparator, period * (1-brightness));
    mcpwm_comparator_set_compare_value(comparator, period * (brightness));

    // go high on counter empty
    mcpwm_generator_set_action_on_timer_event(generator, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    // go low on compare threshold
    mcpwm_generator_set_action_on_compare_event(generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW));

    mcpwm_timer_enable(timer);
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}

void PwmLedStrip::setBrightness(float newBrightness)
{
    brightness = newBrightness;
    // mcpwm_comparator_set_compare_value(comparator, period * (1-brightness));
    mcpwm_comparator_set_compare_value(comparator, period * (brightness));
}
