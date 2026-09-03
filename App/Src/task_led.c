#include "task_led.h"
#include "tasks_config.h"
#include "debugger.h"
#include "led_brightness.h"

#include "platform_pwm.h"

#include "FreeRTOS.h"
#include "queue.h"

#define LED_COMMAND_QUEUE_LENGTH 8U
#define LED_SWEEP_STEP           100U
#define LED_SWEEP_DELAY          20U

typedef struct led_channel_state_t {
    uint8_t target_percent;
    uint16_t target_pulse;
    uint16_t actual_pulse;
} led_channel_state_t;

static QueueHandle_t led_command_queue;
static led_channel_state_t channels[PLATFORM_PWM_CHANNELS_COUNT];
static const char tag[] = "LED";

static void task_led(void* pvParameters);

void task_led_init(void) {
    led_command_queue = xQueueCreate(LED_COMMAND_QUEUE_LENGTH, sizeof(task_led_command_t));

    platform_pwm_init();
    platform_pwm_set_pulse(PLATFORM_PWM_LED1, 0);
    platform_pwm_set_pulse(PLATFORM_PWM_LED2, 0);
    platform_pwm_set_pulse(PLATFORM_PWM_LED3, 0);

    for (int i = 0; i < PLATFORM_PWM_CHANNELS_COUNT; ++i) {
        channels[i].actual_pulse = 0;
        channels[i].target_pulse = 0;
        channels[i].target_percent = 0;
    }
    xTaskCreate(task_led, "LED_TASK", TASK_LED_STACK, NULL, TASK_LED_PRIORITY, NULL);
}

void task_led_set_brightness_percent(platform_pwm_channel_t channel, uint8_t percent) {
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return;
    }
    percent = led_brightness_clamp_percent(percent);

    task_led_command_t command = {.channel = channel, .percent = percent};
    xQueueSend(led_command_queue, &command, 0);
}

uint8_t task_led_get_brightness_percent(platform_pwm_channel_t channel) {
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return 0;
    }
    return channels[(uint8_t)channel].target_percent;
}

uint16_t task_led_get_brightness_pulse(platform_pwm_channel_t channel) {
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return 0;
    }
    return channels[(uint8_t)channel].actual_pulse;
}

static void task_led(void* pvParameters) {

    debugger_log_info(tag, "init completed");

    while (1) {
        task_led_command_t led_command = {0};
        while (xQueueReceive(led_command_queue, &led_command, 0)) {
            led_command.percent = led_brightness_clamp_percent(led_command.percent);

            channels[(uint8_t)led_command.channel].target_percent = led_command.percent;
            channels[(uint8_t)led_command.channel].target_pulse =
                led_brightness_to_pulse(led_command.percent);
        }

        for (int i = 0; i < PLATFORM_PWM_CHANNELS_COUNT; ++i) {
            int32_t channel_diff =
                (int32_t)channels[i].target_pulse - (int32_t)channels[i].actual_pulse;
            if (channel_diff > 0) {
                if (channel_diff < LED_SWEEP_STEP) {
                    channels[i].actual_pulse = channels[i].target_pulse;

                } else {
                    channels[i].actual_pulse += LED_SWEEP_STEP;
                }
            } else if (channel_diff < 0) {
                if (channel_diff > -(int32_t)LED_SWEEP_STEP) {
                    channels[i].actual_pulse = channels[i].target_pulse;

                } else {
                    channels[i].actual_pulse -= LED_SWEEP_STEP;
                }
            }
            platform_pwm_set_pulse(i, channels[i].actual_pulse);
        }
        vTaskDelay(pdMS_TO_TICKS(LED_SWEEP_DELAY));
    }
}