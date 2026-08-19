#include "platform_pwm.h"

#include "tim.h"

static TIM_HandleTypeDef* platform_pwm_timer = &htim3;

void platform_pwm_init() {
    HAL_TIM_PWM_Start(platform_pwm_timer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(platform_pwm_timer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(platform_pwm_timer, TIM_CHANNEL_3);
}

void platform_pwm_set_pulse(platform_pwm_channel_t channel, uint16_t pulse) {
    if (pulse > PLATFORM_PWM_MAX_PULSE) {
        pulse = PLATFORM_PWM_MAX_PULSE;
    }
    switch (channel) {
        case PLATFORM_PWM_LED1:
            __HAL_TIM_SET_COMPARE(platform_pwm_timer, TIM_CHANNEL_1, pulse);
            break;
        case PLATFORM_PWM_LED2:
            __HAL_TIM_SET_COMPARE(platform_pwm_timer, TIM_CHANNEL_2, pulse);
            break;
        case PLATFORM_PWM_LED3:
            __HAL_TIM_SET_COMPARE(platform_pwm_timer, TIM_CHANNEL_3, pulse);
            break;
        default:
            break;
    }
}

uint16_t platform_pwm_get_pulse(platform_pwm_channel_t channel) {
    switch (channel) {
        case PLATFORM_PWM_LED1:
            return __HAL_TIM_GET_COMPARE(platform_pwm_timer, TIM_CHANNEL_1);
        case PLATFORM_PWM_LED2:
            return __HAL_TIM_GET_COMPARE(platform_pwm_timer, TIM_CHANNEL_2);
        case PLATFORM_PWM_LED3:
            return __HAL_TIM_GET_COMPARE(platform_pwm_timer, TIM_CHANNEL_3);
        default:
            return 0;
    }
}
