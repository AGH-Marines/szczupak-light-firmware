#include <stdint.h>
#include <string.h>

#include "task_debug.h"
#include "task_diagnostics.h"
#include "tasks_config.h"

#include "debugger.h"
#include "platform_uart.h"
#include "platform_led.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#if DEBUGGER_ENABLED

#define TASK_DEBUG_DELAY_MS 100U
#define LOG_QUEUE_LENGTH    5

static void task_debug(void* pvParameters);
static void send_registers(void);
static void log_data_to_queue(const uint8_t* data, size_t length);
static void task_debug_uart_tx_completed(void);
static void task_debug_uart_send(const uint8_t* data, size_t length);

static const char tag[] = "DEBUG";
static debugger_command_t current_command;

static QueueHandle_t log_queue;
static SemaphoreHandle_t tx_done;

typedef struct {
    uint8_t length;
    uint8_t data[DEBUGGER_MAX_LOG_FRAME_LENGTH]; /* 179 B — największa ramka przez tę kolejkę */
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
    platform_uart_set_receive_callback(debugger_process_data);
    platform_uart_init();

    const debugger_interface_t interface = {
        .transmit_plot = task_debug_uart_send,
        .transmit_data = log_data_to_queue,
        .get_timestamp_ms = xTaskGetTickCount,
    };
    (void)debugger_init(&interface);

    xTaskCreate(task_debug, "DEBUG_TASK", TASK_DEBUG_STACK, NULL, TASK_DEBUG_PRIORITY, NULL);
}

static void task_debug_uart_tx_completed(void) {
    BaseType_t woken = pdFALSE;
    (void)xSemaphoreGiveFromISR(tx_done, &woken);
    portYIELD_FROM_ISR(woken);
}

static void task_debug_uart_send(const uint8_t* data, size_t length) {
    if (!platform_uart_transmit(data, length)) {
        return;
    }
    (void)xSemaphoreTake(tx_done, portMAX_DELAY);
}

static void send_registers(void) {
    debugger_register_signal(SIGNAL_COUNTER8, DEBUGGER_SIGNAL_TYPE_INT8, "Counter8");
    debugger_register_signal(SIGNAL_LED1, DEBUGGER_SIGNAL_TYPE_UINT32, "LED1 Current");
    debugger_register_signal(SIGNAL_LED2, DEBUGGER_SIGNAL_TYPE_UINT32, "LED2 Current");
    debugger_register_signal(SIGNAL_LED3, DEBUGGER_SIGNAL_TYPE_UINT32, "LED3 Current");
    debugger_register_signal(SIGNAL_12V, DEBUGGER_SIGNAL_TYPE_UINT32, "12V Voltage");
    debugger_register_signal(SIGNAL_USB, DEBUGGER_SIGNAL_TYPE_UINT32, "USB Voltage");
}

static void log_data_to_queue(const uint8_t* data, size_t length) {
    if (length > DEBUGGER_MAX_LOG_FRAME_LENGTH) {
        return;
    }
    debug_log_frame_t frame;

    frame.length = (uint8_t)length;
    memcpy(frame.data, data, length);
    if (log_queue != NULL) {
        xQueueSend(log_queue, &frame, 0);
    }
}

static void task_debug(void* pvParameters) {
    TickType_t last_wake = xTaskGetTickCount();

    task_diagnostics_measurement_frame_t measurement_frame = {0};
    debugger_log_info(tag, "init completed");
    send_registers();

    while (1) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_DEBUG_DELAY_MS));

        debug_log_frame_t msg;

        while (log_queue != NULL && xQueueReceive(log_queue, &msg, 0) == pdTRUE) {
            task_debug_uart_send(msg.data, msg.length);
        }
        QueueHandle_t measurement_queue = task_diagnostics_get_measurement_queue();

        if (measurement_queue != NULL &&
            xQueuePeek(measurement_queue, &measurement_frame, 0) == pdTRUE) {
            debugger_plot_signal(SIGNAL_LED1, &measurement_frame.current_led1);
            debugger_plot_signal(SIGNAL_LED2, &measurement_frame.current_led2);
            debugger_plot_signal(SIGNAL_LED3, &measurement_frame.current_led3);
            debugger_plot_signal(SIGNAL_12V, &measurement_frame.voltage_12v);
            debugger_plot_signal(SIGNAL_USB, &measurement_frame.voltage_usb);
        }
    }
}

#endif /* DEBUGGER_ENABLED */
