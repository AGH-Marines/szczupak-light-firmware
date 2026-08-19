/*
 *  platform_pwm.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_PWM_H
#define PLATFORM_PWM_H

#include <stdint.h>

#define PLATFORM_PWM_MAX_PULSE      9999U
#define PLATFORM_PWM_CHANNELS_COUNT 3U

typedef enum platform_pwm_channel_t {
    PLATFORM_PWM_LED1,
    PLATFORM_PWM_LED2,
    PLATFORM_PWM_LED3
} platform_pwm_channel_t;

void platform_pwm_init();
void platform_pwm_set_pulse(platform_pwm_channel_t channel, uint16_t pulse);
uint16_t platform_pwm_get_pulse(platform_pwm_channel_t channel);

#endif /* PLATFORM_PWM_H */