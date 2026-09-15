#include "task_led.h"
#include "tasks_config.h"
#include "task_diagnostics.h"

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

static volatile bool led_enabled = false;

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
    configASSERT(xTaskCreate(task_led, "LED_TASK", TASK_LED_STACK, NULL, TASK_LED_PRIORITY, NULL) ==
                 pdPASS);
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

bool task_led_channel_ready(platform_pwm_channel_t channel) {
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return 0;
    }
    return channels[(uint8_t)channel].actual_pulse == channels[(uint8_t)channel].target_pulse;
}

void task_led_set_enabled(bool enable) {
    led_enabled = enable;
}

bool task_led_get_enabled() {
    return led_enabled;
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

        QueueHandle_t fault_queue = task_diagnostics_get_fault_queue();
        if (fault_queue == NULL) {
            return;
        }
        task_diagnostics_fault_frame_t fault_frame = {0};
        if (xQueuePeek(fault_queue, &fault_frame, 0) == pdTRUE) {
            bool faults_overcurrent[3] = {
                fault_frame.led1_overcurrent,
                fault_frame.led2_overcurrent,
                fault_frame.led3_overcurrent,
            };
            bool faults_noload[3] = {
                fault_frame.led1_noload,
                fault_frame.led2_noload,
                fault_frame.led3_noload,

            };
            for (int i = 0; i < PLATFORM_PWM_CHANNELS_COUNT; ++i) {
                if (faults_noload[i] || faults_overcurrent[i]) {
                    channels[i].actual_pulse = 0U;
                    platform_pwm_set_pulse(i, channels[i].actual_pulse);
                    continue;
                }
                uint16_t target_pulse = led_enabled ? channels[i].target_pulse : 0U;
                int32_t channel_diff = (int32_t)target_pulse - (int32_t)channels[i].actual_pulse;
                if (channel_diff > 0) {
                    if (channel_diff < LED_SWEEP_STEP) {
                        channels[i].actual_pulse = target_pulse;

                    } else {
                        channels[i].actual_pulse += LED_SWEEP_STEP;
                    }
                } else if (channel_diff < 0) {
                    if (channel_diff > -(int32_t)LED_SWEEP_STEP) {
                        channels[i].actual_pulse = target_pulse;

                    } else {
                        channels[i].actual_pulse -= LED_SWEEP_STEP;
                    }
                }
                platform_pwm_set_pulse(i, channels[i].actual_pulse);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(LED_SWEEP_DELAY));
    }
}