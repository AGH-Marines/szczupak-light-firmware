#include "can.h"
#include "debugger.h"

#include "task_can.h"
#include "task_led.h"
#include "task_diagnostics.h"
#include "tasks_config.h"

#include "platform_can.h"
#include "platform_pwm.h"
#include "platform_boot.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TASK_CAN_QUEUE_SIZE 8U
#define TASK_CAN_DELAY_MS   100U

static const char tag[] = "CAN";
static QueueHandle_t receive_queue;
static TaskHandle_t task_can_handle;
static bool fault_reset_z1 = false;

static void task_can(void* pvParameters);
static void task_can_receive_callback(const can_frame_t* frame);
static void task_can_handle_frame(const can_frame_t* frame);
static void task_can_send_status();
static void task_can_reset_callback();

void task_can_init(void) {
    receive_queue = xQueueCreate(TASK_CAN_QUEUE_SIZE, sizeof(can_frame_t));
    configASSERT(receive_queue);
    platform_can_set_receive_callback(task_can_receive_callback);
    platform_can_set_reset_callback(task_can_reset_callback);
    configASSERT(xTaskCreate(task_can, "CAN_TASK", TASK_CAN_STACK, NULL, TASK_CAN_PRIORITY,
                             &task_can_handle) == pdPASS);
    platform_can_init();
}

static void task_can_receive_callback(const can_frame_t* frame) {
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(receive_queue, frame, &woken);
    portYIELD_FROM_ISR(woken);
}

static void task_can_reset_callback() {
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(task_can_handle, &woken);
    portYIELD_FROM_ISR(woken);
}

static void task_can_handle_frame(const can_frame_t* frame) {
    can_boot_command_t boot_cmd;
    if (can_decode_boot_command(frame, &boot_cmd)) {
        if ((boot_cmd.target_mask & (1U << LIGHT_BOOT_NODE_ID)) == 0U) {
            return;
        }
        switch (boot_cmd.type) {
            case CAN_BOOT_COMMAND_ENTER_UPDATE: {
                platform_boot_request_set();
                platform_boot_reset();
                break;
            }
            case CAN_BOOT_COMMAND_STATUS_REQUEST: {
                can_boot_status_t status = {
                    .device_id = LIGHT_BOOT_NODE_ID,
                    .type = CAN_BOOT_STATUS_APP_READY,
                };
                can_frame_t status_frame = {0};

                if (can_encode_boot_status(&status, &status_frame)) {
                    if (!platform_can_transmit(&status_frame)) {
                        debugger_log_warning(tag, "could not transmit boot status frame.");
                    }
                }

                break;
            }
            case CAN_BOOT_COMMAND_RESET: {
                platform_boot_reset();
                break;
            }
            default:
                break;
        }

        return;
    }

    can_command_t cmd;

    if (!can_decode_command(frame, &cmd)) {
        return;
    }

    const uint8_t led_percent[3] = {
        cmd.led1_percent,
        cmd.led2_percent,
        cmd.led3_percent,
    };

    for (uint8_t i = 0; i < 3; ++i) {
        uint8_t percent = led_percent[i];
        task_led_set_brightness_percent((platform_pwm_channel_t)i, percent);
    }
    task_led_set_enabled(cmd.enable);

    if (cmd.fault_reset == true && fault_reset_z1 == false) {
        task_diagnostics_clear_faults();
    }
    fault_reset_z1 = cmd.fault_reset;
}

static void task_can_send_status() {
    QueueHandle_t measurement_queue = task_diagnostics_get_measurement_queue();
    QueueHandle_t fault_queue = task_diagnostics_get_fault_queue();

    task_diagnostics_measurement_frame_t measurement = {0};
    if ((measurement_queue == NULL) || (fault_queue == NULL)) {
        return;
    }
    if (xQueuePeek(measurement_queue, &measurement, 0) == pdTRUE) {
        uint8_t led_percent[3] = {0};
        for (uint8_t i = 0; i < 3; ++i) {
            led_percent[i] = task_led_get_brightness_percent((platform_pwm_channel_t)i);
        }

        bool enable = task_led_get_enabled();
        can_power_status_t power_status = {
            .usb_voltage_mv = measurement.voltage_usb,
            .power_voltage_mv = measurement.voltage_12v,
            .led1_percent = led_percent[0],
            .led2_percent = led_percent[1],
            .led3_percent = led_percent[2],
            .enable = enable,
        };
        can_current_status_t current_status = {
            .led1_current_ma = measurement.current_led1,
            .led2_current_ma = measurement.current_led2,
            .led3_current_ma = measurement.current_led3,
        };
        can_frame_t power_frame = {0};
        can_frame_t current_frame = {0};

        if (can_encode_power_status(&power_status, &power_frame)) {
            if (!platform_can_transmit(&power_frame)) {
                debugger_log_warning(tag, "could not transmit power status frame.");
            }
        }
        if (can_encode_current_status(&current_status, &current_frame)) {
            if (!platform_can_transmit(&current_frame)) {
                debugger_log_warning(tag, "could not transmit current status frame.");
            }
        }
    }
    task_diagnostics_fault_frame_t fault_frame = {0};
    if (xQueuePeek(fault_queue, &fault_frame, 0) == pdTRUE) {

        can_fault_status_t fault_status = {
            .led1_overcurrent = fault_frame.led1_overcurrent,
            .led2_overcurrent = fault_frame.led2_overcurrent,
            .led3_overcurrent = fault_frame.led3_overcurrent,
            .led1_no_load = fault_frame.led1_noload,
            .led2_no_load = fault_frame.led2_noload,
            .led3_no_load = fault_frame.led3_noload,
            .usb_undervoltage = fault_frame.usb_undervoltage,
            .power_undervoltage = fault_frame.power_undervoltage,
        };
        can_frame_t fault_can_frame = {0};

        if (can_encode_fault_status(&fault_status, &fault_can_frame)) {
            if (!platform_can_transmit(&fault_can_frame)) {
                debugger_log_warning(tag, "could not transmit fault status frame.");
            }
        }
    }
}

static void task_can(void* pvParameters) {

    debugger_log_info(tag, "init completed");

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TASK_CAN_DELAY_MS))) {
            platform_can_reset();
            debugger_log_warning(tag, "BUS_OFF, resetting.");
        }
        can_frame_t frame;
        while (xQueueReceive(receive_queue, &frame, 0) == pdTRUE) {
            task_can_handle_frame(&frame);
        }
        task_can_send_status();
    }
}