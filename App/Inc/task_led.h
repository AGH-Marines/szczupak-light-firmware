/*
 *  task_led.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef TASK_LED_H
#define TASK_LED_H

#include "platform_pwm.h"

typedef struct task_led_command_t {
    platform_pwm_channel_t channel;
    uint16_t pulse;
} task_led_command_t;

void task_led_init(void);
void task_led_set_pulse(platform_pwm_channel_t channel, uint16_t pulse);
uint16_t task_led_get_pulse(platform_pwm_channel_t channel);

#endif /* TASK_LED_H */