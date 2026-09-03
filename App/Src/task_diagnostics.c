#include "tasks_config.h"
#include "task_diagnostics.h"

#include "voltage_sense.h"
#include "current_sense.h"
#include "debugger.h"

#include "platform_adc.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

static void diagnostics_task(void* pvParameters);
static void task_diagnostics_adc_notify();

#define VREF      3300U
#define ADC_MAX   4095U
#define RHIGH_12V 100000U
#define RLOW_12V  10000U
#define RHIGH_USB 100000U
#define RLOW_USB  100000U

#define RSHUNT_M0HM 10U
#define AMP_GAIN    100U

static const char tag[] = "DIAG";

static voltage_sense_t voltage_sense_12v = {
    .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF, ADC_MAX, RHIGH_12V, RLOW_12V)};
static voltage_sense_t voltage_sense_usb = {
    .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF, ADC_MAX, RHIGH_USB, RLOW_USB)};

static current_sense_t current_sense_led = {
    .scale_q16 = CURRENT_SENSE_CALC_SCALE(VREF, ADC_MAX, RSHUNT_M0HM, AMP_GAIN)};

static TaskHandle_t diagnostics_task_handle;

static QueueHandle_t measurement_queue_handle;

QueueHandle_t task_diagnostics_get_measurement_queue() {
    return measurement_queue_handle;
}

void task_diagnostics_init() {
    measurement_queue_handle = xQueueCreate(1, sizeof(task_diagnostics_measurement_frame_t));
    configASSERT(measurement_queue_handle != NULL);

    configASSERT(xTaskCreate(diagnostics_task, "diag_task", TASK_DIAGNOSTICS_STACK, NULL,
                             TASK_DIAGNOSTICS_PRIORITY, NULL) == pdPASS);
}

static void task_diagnostics_adc_notify() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(diagnostics_task_handle, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void diagnostics_task(void* pvParameters) {
    diagnostics_task_handle = xTaskGetCurrentTaskHandle();
    platform_adc_set_callback(task_diagnostics_adc_notify);
    platform_adc_init();

    debugger_log_info(tag, "init completed");

    while (1) {
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        platform_adc_sample_t sample = platform_adc_get();

        uint32_t voltage_12v = voltage_sense_to_mv(&voltage_sense_12v, sample.adc_12v);
        uint32_t voltage_usb = voltage_sense_to_mv(&voltage_sense_usb, sample.adc_usb);

        uint32_t current_led1 = current_sense_to_ma(&current_sense_led, sample.adc_led1);
        uint32_t current_led2 = current_sense_to_ma(&current_sense_led, sample.adc_led2);
        uint32_t current_led3 = current_sense_to_ma(&current_sense_led, sample.adc_led3);

        task_diagnostics_measurement_frame_t frame = {.current_led1 = current_led1,
                                                      .current_led2 = current_led2,
                                                      .current_led3 = current_led3,
                                                      .voltage_12v = voltage_12v,
                                                      .voltage_usb = voltage_usb};

        xQueueOverwrite(measurement_queue_handle, &frame);
    }
}
