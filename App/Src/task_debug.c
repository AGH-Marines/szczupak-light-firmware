#include <stdint.h>
#include <string.h>

#include "task_debug.h"
#include "task_diagnostics.h"
#include "task_led.h"
#include "tasks_config.h"

#include "debugger.h"
#include "frame.h"

#include "platform_uart.h"
#include "platform_led.h"
#include "platform_pwm.h"
#include "platform_reset.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#if DEBUGGER_ENABLED

#define TASK_DEBUG_DELAY_MS      100U
#define LOG_QUEUE_LENGTH         5U
#define UART_TRANSMIT_TIMEOUT_MS 50U

static void task_debug(void* pvParameters);
static void send_registers(void);
static void task_debug_uart_tx_completed(void);
static void task_debug_uart_rx(const uint8_t* data, size_t length);
static void task_debug_transmit(uint8_t frame_type, const uint8_t* payload, uint8_t length);
static void task_debug_uart_send(const uint8_t* data, size_t length);
static void task_debug_handle_command(const frame_t* frame);

static const char tag[] = "DEBUG";

static frame_t command_queue[FRAME_COMMAND_QUEUE_SIZE];
static frame_parser_t parser;

static QueueHandle_t log_queue;
static SemaphoreHandle_t tx_done;

typedef struct {
    uint8_t frame_type;
    uint8_t length;
    uint8_t payload[3U + 4U + DEBUGGER_MAX_PREFIX_LENGTH + DEBUGGER_MAX_TAG_LENGTH +
                    DEBUGGER_MAX_MESSAGE_LENGTH];
} debug_log_frame_t;

void task_debug_init() {
    log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(debug_log_frame_t));
    if (log_queue == NULL) {
        platform_led_fault_set(true);
        while (1)
            ;
    }
    tx_done = xSemaphoreCreateBinary();
    platform_uart_set_transmit_complete_callback(task_debug_uart_tx_completed);
    (void)frame_parser_init(&parser, command_queue, FRAME_COMMAND_QUEUE_SIZE);
    platform_uart_set_receive_callback(task_debug_uart_rx);
    platform_uart_init();

    const debugger_interface_t interface = {
        .transmit = task_debug_transmit,
        .get_timestamp_ms = xTaskGetTickCount,
    };
    (void)debugger_init(&interface);

    configASSERT(xTaskCreate(task_debug, "DEBUG_TASK", TASK_DEBUG_STACK, NULL, TASK_DEBUG_PRIORITY,
                             NULL) == pdPASS);
}

static void task_debug_uart_tx_completed(void) {
    BaseType_t woken = pdFALSE;
    (void)xSemaphoreGiveFromISR(tx_done, &woken);
    portYIELD_FROM_ISR(woken);
}

static void task_debug_uart_rx(const uint8_t* data, size_t length) {
    frame_decode(&parser, data, length);
}

static void task_debug_uart_send(const uint8_t* data, size_t length) {
    if (!platform_uart_transmit(data, length)) {
        return;
    }
    if (xSemaphoreTake(tx_done, pdMS_TO_TICKS(UART_TRANSMIT_TIMEOUT_MS)) == pdFALSE) {
        platform_uart_abort_transmission();
    }
}

static void send_registers(void) {
    debugger_register_signal(SIGNAL_COUNTER8, DEBUGGER_SIGNAL_TYPE_INT8, "Counter8");
    debugger_register_signal(SIGNAL_LED1, DEBUGGER_SIGNAL_TYPE_UINT32, "LED1 Current mA");
    debugger_register_signal(SIGNAL_LED2, DEBUGGER_SIGNAL_TYPE_UINT32, "LED2 Current mA");
    debugger_register_signal(SIGNAL_LED3, DEBUGGER_SIGNAL_TYPE_UINT32, "LED3 Current mA");
    debugger_register_signal(SIGNAL_12V, DEBUGGER_SIGNAL_TYPE_UINT32, "12V Voltage mV");
    debugger_register_signal(SIGNAL_USB, DEBUGGER_SIGNAL_TYPE_UINT32, "USB Voltage mV");
    debugger_register_signal(SIGNAL_PWM_LED1, DEBUGGER_SIGNAL_TYPE_UINT8, "LED1 PWM %");
    debugger_register_signal(SIGNAL_PWM_LED2, DEBUGGER_SIGNAL_TYPE_UINT8, "LED2 PWM %");
    debugger_register_signal(SIGNAL_PWM_LED3, DEBUGGER_SIGNAL_TYPE_UINT8, "LED3 PWM %");
}

static void task_debug_transmit(uint8_t frame_type, const uint8_t* payload, uint8_t length) {
    if (frame_type == (uint8_t)DEBUGGER_FRAME_TYPE_LOG) {
        debug_log_frame_t frame = {.frame_type = frame_type, .length = length};
        if (length > sizeof(frame.payload)) {
            return;
        }
        memcpy(frame.payload, payload, length);
        (void)xQueueSend(log_queue, &frame, 0);
        return;
    }
    uint8_t data[5U + 3U + DEBUGGER_MAX_SIGNAL_NAME_LENGTH];
    const size_t encoded = frame_encode(data, frame_type, payload, length);
    if (encoded == 0U) {
        return;
    }
    task_debug_uart_send(data, encoded);
}

static void task_debug_handle_command(const frame_t* frame) {
    if (frame == NULL) {
        return;
    }
    switch (frame->command_type) {
        case APP_COMMAND_REQUEST_REGISTERS: {
            send_registers();
            break;
        }
        case APP_COMMAND_SET_LED: {
            if (frame->length == 2) {
                uint8_t led = frame->payload[0];
                uint8_t percent = frame->payload[1];
                if ((led < 1) || (led > 3) || (percent > 100)) {
                    return;
                }
                task_led_set_brightness_percent((platform_pwm_channel_t)(led - 1), percent);
            }
            break;
        }
        default:
            break;
    }
}

static void task_debug(void* pvParameters) {
    TickType_t last_wake = xTaskGetTickCount();

    task_diagnostics_measurement_frame_t measurement_frame = {0};
    debugger_log_reset(tag, platform_reset_reason());
    send_registers();

    debugger_log_info(tag, "init completed");

    while (1) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_DEBUG_DELAY_MS));
        frame_t frame;
        while (frame_parser_get(&parser, &frame)) {
            task_debug_handle_command(&frame);
        }

        debug_log_frame_t msg;
        while (log_queue != NULL && xQueueReceive(log_queue, &msg, 0) == pdTRUE) {
            uint8_t data[5U + sizeof(msg.payload)];
            const size_t encoded = frame_encode(data, msg.frame_type, msg.payload, msg.length);
            if (encoded != 0U) {
                task_debug_uart_send(data, encoded);
            }
        }

        QueueHandle_t measurement_queue = task_diagnostics_get_measurement_queue();
        if (measurement_queue != NULL &&
            xQueuePeek(measurement_queue, &measurement_frame, 0) == pdTRUE) {
            debugger_plot_signal(SIGNAL_LED1, &measurement_frame.current_led1);
            debugger_plot_signal(SIGNAL_LED2, &measurement_frame.current_led2);
            debugger_plot_signal(SIGNAL_LED3, &measurement_frame.current_led3);
            debugger_plot_signal(SIGNAL_12V, &measurement_frame.voltage_12v);
            debugger_plot_signal(SIGNAL_USB, &measurement_frame.voltage_usb);

            uint8_t led1_percentage = task_led_get_brightness_percent(PLATFORM_PWM_LED1);
            uint8_t led2_percentage = task_led_get_brightness_percent(PLATFORM_PWM_LED2);
            uint8_t led3_percentage = task_led_get_brightness_percent(PLATFORM_PWM_LED3);

            debugger_plot_signal(SIGNAL_PWM_LED1, &led1_percentage);
            debugger_plot_signal(SIGNAL_PWM_LED2, &led2_percentage);
            debugger_plot_signal(SIGNAL_PWM_LED3, &led3_percentage);
        }
    }
}

#endif /* DEBUGGER_ENABLED */
