#include <string.h>

#include "task_usb.h"
#include "task_diagnostics.h"
#include "task_led.h"
#include "tasks_config.h"
#include "app_command.h"

#include "frame.h"
#include "debugger.h"

#include "platform_pwm.h"
#include "platform_usb.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define TASK_USB_DELAY_MS 10U

static const char tag[] = "USB";

static frame_t command_queue[FRAME_COMMAND_QUEUE_SIZE];
static frame_parser_t parser;
static SemaphoreHandle_t tx_done;

static void task_usb(void* pvParameters);
static void task_usb_rx(const uint8_t* data, size_t length);
static void task_usb_tx_completed(void);
static void task_usb_send(const uint8_t* data, size_t length);
static void task_usb_handle_command(const frame_t* frame);

void task_usb_init(void) {
    (void)frame_parser_init(&parser, command_queue, FRAME_COMMAND_QUEUE_SIZE);
    tx_done = xSemaphoreCreateBinary();
    platform_usb_set_receive_callback(task_usb_rx);
    platform_usb_set_transmit_complete_callback(task_usb_tx_completed);
    xTaskCreate(task_usb, "USB_TASK", TASK_DEBUG_STACK, NULL, TASK_USB_PRIORITY, NULL);
}

static void task_usb_rx(const uint8_t* data, size_t length) {
    frame_decode(&parser, data, length);
}

static void task_usb_tx_completed(void) {
    BaseType_t woken = pdFALSE;
    (void)xSemaphoreGiveFromISR(tx_done, &woken);
    portYIELD_FROM_ISR(woken);
}

static void task_usb_send(const uint8_t* data, size_t length) {
    if (!platform_usb_transmit(data, length)) {
        return;
    }
    (void)xSemaphoreTake(tx_done, portMAX_DELAY);
}

static void task_usb_handle_command(const frame_t* frame) {
    if (frame == NULL) {
        return;
    }
    switch (frame->command_type) {
        case APP_COMMAND_GET_MEASUREMENTS: {
            QueueSetHandle_t queue = task_diagnostics_get_measurement_queue();
            task_diagnostics_measurement_frame_t measurement = {0};
            if ((queue == NULL) || (xQueuePeek(queue, &measurement, 0) != pdTRUE)) {
                return;
            }
            uint8_t payload[sizeof(measurement)];
            memcpy(payload, &measurement, sizeof(measurement));

            uint8_t data[5U + sizeof(payload)];
            const size_t encoded =
                frame_encode(data, APP_COMMAND_GET_MEASUREMENTS, payload, (uint8_t)sizeof(payload));
            if (encoded == 0U) {
                return;
            }
            task_usb_send(data, encoded);
        }
        case APP_COMMAND_SET_LED: {
            if (frame->length != 2U) {
                break;
            }
            const uint8_t led = frame->payload[0];
            const uint8_t percent = frame->payload[1];
            if ((led < 1U) || (led > 3U) || (percent > 100U)) {
                break;
            }
            const uint16_t pulse =
                (uint16_t)((uint32_t)percent * (uint32_t)PLATFORM_PWM_MAX_PULSE / (uint32_t)100U);
            task_led_set_pulse((platform_pwm_channel_t)(led - 1U), pulse);

            uint8_t data[5U + 2U];
            const size_t encoded = frame_encode(data, APP_COMMAND_SET_LED, frame->payload, 2U);
            if (encoded != 0U) {
                task_usb_send(data, encoded);
            }
            break;
        }
        case APP_COMMAND_GET_LED: {
            uint8_t payload[3];
            for (uint8_t i = 0U; i < 3U; ++i) {
                const uint16_t pulse = task_led_get_pulse((platform_pwm_channel_t)i);
                payload[i] = (uint8_t)(((uint32_t)pulse * 100U) / (uint32_t)PLATFORM_PWM_MAX_PULSE);
            }
            uint8_t data[5U + 3U];
            const size_t encoded = frame_encode(data, APP_COMMAND_GET_LED, payload, 3U);
            if (encoded != 0U) {
                task_usb_send(data, encoded);
            }
        }

        default:
            break;
    }
}

static void task_usb(void* pvParameters) {
    debugger_log_info(tag, "init completed.");
    while (1) {
        frame_t frame;
        while (frame_parser_get(&parser, &frame)) {
            task_usb_handle_command(&frame);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_USB_DELAY_MS));
    }
}
