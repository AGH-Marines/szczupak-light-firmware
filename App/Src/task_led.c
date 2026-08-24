#include "task_led.h"
#include "tasks_config.h"
#include "debugger.h"

#include "platform_pwm.h"

#include "FreeRTOS.h"
#include "queue.h"

#define LED_COMMAND_QUEUE_LENGTH 8U
#define LED_SWEEP_STEP           100U
#define LED_SWEEP_DELAY          20U

typedef struct led_channel_state_t {
    uint16_t target;
    uint16_t actual;
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
        channels[i].actual = 0;
        channels[i].target = 0;
    }
    xTaskCreate(task_led, "LED_TASK", TASK_LED_STACK, NULL, TASK_LED_PRIORITY, NULL);
}

void task_led_set_pulse(platform_pwm_channel_t channel, uint16_t pulse) {
    if (pulse > PLATFORM_PWM_MAX_PULSE) {
        pulse = PLATFORM_PWM_MAX_PULSE;
    }
    task_led_command_t command = {.channel = channel, .pulse = pulse};
    xQueueSend(led_command_queue, &command, 0);
}

uint16_t task_led_get_pulse(platform_pwm_channel_t channel) {
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return 0;
    }
    return channels[(uint8_t)channel].actual;
}

static void task_led(void* pvParameters) {
    debugger_log_info(tag, "init completed");

    while (1) {
        task_led_command_t led_command = {0};
        while (xQueueReceive(led_command_queue, &led_command, 0)) {
            if (led_command.pulse > PLATFORM_PWM_MAX_PULSE) {
                led_command.pulse = PLATFORM_PWM_MAX_PULSE;
            }
            channels[(uint8_t)led_command.channel].target = led_command.pulse;
        }

        for (int i = 0; i < PLATFORM_PWM_CHANNELS_COUNT; ++i) {
            int32_t channel_diff = (int32_t)channels[i].target - (int32_t)channels[i].actual;
            if (channel_diff > 0) {
                if (channel_diff < LED_SWEEP_STEP) {
                    channels[i].actual = channels[i].target;

                } else {
                    channels[i].actual += LED_SWEEP_STEP;
                }
            } else if (channel_diff < 0) {
                if (channel_diff > -(int32_t)LED_SWEEP_STEP) {
                    channels[i].actual = channels[i].target;

                } else {
                    channels[i].actual -= LED_SWEEP_STEP;
                }
            }
            platform_pwm_set_pulse(i, channels[i].actual);
        }
        vTaskDelay(pdMS_TO_TICKS(LED_SWEEP_DELAY));
    }
}