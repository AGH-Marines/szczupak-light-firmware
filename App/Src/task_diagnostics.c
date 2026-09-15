#include "tasks_config.h"
#include "task_diagnostics.h"
#include "task_led.h"

#include "voltage_sense.h"
#include "current_sense.h"
#include "debugger.h"

#include "platform_adc.h"
#include "platform_pwm.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

static void diagnostics_task(void* pvParameters);
static void task_diagnostics_adc_notify();

#define VREF        3300U
#define ADC_MAX     4095U
#define RHIGH_12V   100000U
#define RLOW_12V    10000U
#define RHIGH_USB   100000U
#define RLOW_USB    100000U
#define RSHUNT_M0HM 10U
#define AMP_GAIN    100U

#define FAULT_DEBOUNCE_SAMPLES             32U
#define POWER_UNDERVOLTAGE_UPPER_THRESHOLD 11000U
#define POWER_UNDERVOLTAGE_LOWER_THRESHOLD 10500U
#define USB_UNDERVOLTAGE_UPPER_THRESHOLD   4000U
#define USB_UNDERVOLTAGE_LOWER_THRESHOLD   3900U

#define LED_CURRENT_MAX_MA  850U
#define LED_FAULT_PWM_STEP  10U
#define LED_FAULT_TABLE_LEN 11U

static const char tag[] = "DIAG";

static voltage_sense_t voltage_sense_12v = {
    .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF, ADC_MAX, RHIGH_12V, RLOW_12V)};
static voltage_sense_t voltage_sense_usb = {
    .scale_q16 = VOLTAGE_SENSE_CALC_SCALE(VREF, ADC_MAX, RHIGH_USB, RLOW_USB)};
static current_sense_t current_sense_led = {
    .scale_q16 = CURRENT_SENSE_CALC_SCALE(VREF, ADC_MAX, RSHUNT_M0HM, AMP_GAIN)};

static const uint16_t led_min_current_ma[LED_FAULT_TABLE_LEN] = {
    0, 0, 0, 14, 29, 50, 79, 111, 151, 200, 252,
};

static const uint16_t led_max_current_ma[LED_FAULT_TABLE_LEN] = {
    0, 0, 0, 58, 117, 200, 315, 443, 603, 800, 850,
};

static TaskHandle_t diagnostics_task_handle;
static QueueHandle_t measurement_queue_handle;
static QueueHandle_t fault_queue_handle;

volatile bool clear_faults_flag = false;

typedef struct fault_debounce_t {
    bool active;
    uint16_t count;
} fault_debounce_t;

static fault_debounce_t fault_debounce_power_undervoltage;
static fault_debounce_t fault_debounce_usb_undervoltage;
static fault_debounce_t fault_debounce_led1_overcurrent;
static fault_debounce_t fault_debounce_led2_overcurrent;
static fault_debounce_t fault_debounce_led3_overcurrent;
static fault_debounce_t fault_debounce_led1_noload;
static fault_debounce_t fault_debounce_led2_noload;
static fault_debounce_t fault_debounce_led3_noload;

static uint8_t led_fault_table_index(uint8_t percent) {
    if (percent > 100U) {
        percent = 100U;
    }
    return (uint8_t)(percent / LED_FAULT_PWM_STEP);
}

QueueHandle_t task_diagnostics_get_measurement_queue() {
    return measurement_queue_handle;
}

QueueHandle_t task_diagnostics_get_fault_queue() {
    return fault_queue_handle;
}

void task_diagnostics_init() {
    measurement_queue_handle = xQueueCreate(1, sizeof(task_diagnostics_measurement_frame_t));
    configASSERT(measurement_queue_handle != NULL);
    fault_queue_handle = xQueueCreate(1, sizeof(task_diagnostics_fault_frame_t));
    configASSERT(fault_queue_handle != NULL);

    configASSERT(xTaskCreate(diagnostics_task, "diag_task", TASK_DIAGNOSTICS_STACK, NULL,
                             TASK_DIAGNOSTICS_PRIORITY, NULL) == pdPASS);
}

void task_diagnostics_clear_faults() {
    clear_faults_flag = true;
}

static void task_diagnostics_adc_notify() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(diagnostics_task_handle, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void fault_debounce_update(fault_debounce_t* debounce, bool condition) {
    if (debounce == NULL) {
        return;
    }
    if (condition != debounce->active) {
        debounce->count++;
    } else if (condition == debounce->active) {
        debounce->count = 0U;
    }
    if (debounce->count >= FAULT_DEBOUNCE_SAMPLES) {
        debounce->active = condition;
        debounce->count = 0U;
    }
}

static void fault_debounce_clear(fault_debounce_t* debounce) {
    if (debounce == NULL) {
        return;
    }
    debounce->active = false;
    debounce->count = 0U;
}

static bool fault_undervoltage_condition(const fault_debounce_t* debounce, uint32_t voltage_mv,
                                         uint32_t upper_threshold, uint32_t lower_threshold) {
    if (debounce->active) {
        return voltage_mv < upper_threshold;
    }
    return voltage_mv < lower_threshold;
}

static bool fault_overcurrent_condition(const fault_debounce_t* debounce, uint32_t current_ma,
                                        platform_pwm_channel_t channel) {
    if (debounce->active) {
        return true;
    }
    if (current_ma > LED_CURRENT_MAX_MA) {
        return true;
    }
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return false;
    }
    if (!task_led_channel_ready(channel)) {
        return false;
    }
    const uint8_t percent = task_led_get_brightness_percent(channel);

    const uint16_t max_ma = led_max_current_ma[led_fault_table_index(percent)];
    if (max_ma == 0U) {
        return false;
    }
    return (current_ma > max_ma);
}

static bool fault_noload_condition(const fault_debounce_t* debounce, uint32_t current_ma,
                                   platform_pwm_channel_t channel) {
    if (debounce->active) {
        return true;
    }
    if (channel >= PLATFORM_PWM_CHANNELS_COUNT) {
        return false;
    }
    if (!task_led_channel_ready(channel)) {
        return false;
    }
    const uint8_t percent = task_led_get_brightness_percent(channel);

    const uint16_t min_ma = led_min_current_ma[led_fault_table_index(percent)];
    if (min_ma == 0U) {
        return false;
    }

    return (current_ma < min_ma);
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

        if (clear_faults_flag) {
            fault_debounce_clear(&fault_debounce_usb_undervoltage);
            fault_debounce_clear(&fault_debounce_power_undervoltage);
            fault_debounce_clear(&fault_debounce_led1_overcurrent);
            fault_debounce_clear(&fault_debounce_led2_overcurrent);
            fault_debounce_clear(&fault_debounce_led3_overcurrent);
            fault_debounce_clear(&fault_debounce_led1_noload);
            fault_debounce_clear(&fault_debounce_led2_noload);
            fault_debounce_clear(&fault_debounce_led3_noload);
            clear_faults_flag = false;
        }

        bool power_condition = fault_undervoltage_condition(
            &fault_debounce_power_undervoltage, voltage_12v, POWER_UNDERVOLTAGE_UPPER_THRESHOLD,
            POWER_UNDERVOLTAGE_LOWER_THRESHOLD);
        fault_debounce_update(&fault_debounce_power_undervoltage, power_condition);
        bool usb_condition = fault_undervoltage_condition(
            &fault_debounce_usb_undervoltage, voltage_usb, USB_UNDERVOLTAGE_UPPER_THRESHOLD,
            USB_UNDERVOLTAGE_LOWER_THRESHOLD);
        fault_debounce_update(&fault_debounce_usb_undervoltage, usb_condition);

        bool led1_overcurrent = fault_overcurrent_condition(&fault_debounce_led1_overcurrent,
                                                            current_led1, PLATFORM_PWM_LED1);
        fault_debounce_update(&fault_debounce_led1_overcurrent, led1_overcurrent);
        bool led2_overcurrent = fault_overcurrent_condition(&fault_debounce_led2_overcurrent,
                                                            current_led2, PLATFORM_PWM_LED2);
        fault_debounce_update(&fault_debounce_led2_overcurrent, led2_overcurrent);
        bool led3_overcurrent = fault_overcurrent_condition(&fault_debounce_led3_overcurrent,
                                                            current_led3, PLATFORM_PWM_LED3);
        fault_debounce_update(&fault_debounce_led3_overcurrent, led3_overcurrent);

        bool led1_noload =
            fault_noload_condition(&fault_debounce_led1_noload, current_led1, PLATFORM_PWM_LED1);
        fault_debounce_update(&fault_debounce_led1_noload, led1_noload);
        bool led2_noload =
            fault_noload_condition(&fault_debounce_led2_noload, current_led2, PLATFORM_PWM_LED2);
        fault_debounce_update(&fault_debounce_led2_noload, led2_noload);
        bool led3_noload =
            fault_noload_condition(&fault_debounce_led3_noload, current_led3, PLATFORM_PWM_LED3);
        fault_debounce_update(&fault_debounce_led3_noload, led3_noload);

        task_diagnostics_fault_frame_t fault = {0};
        fault.power_undervoltage = fault_debounce_power_undervoltage.active;
        fault.usb_undervoltage = fault_debounce_usb_undervoltage.active;
        fault.led1_overcurrent = fault_debounce_led1_overcurrent.active;
        fault.led2_overcurrent = fault_debounce_led2_overcurrent.active;
        fault.led3_overcurrent = fault_debounce_led3_overcurrent.active;
        fault.led1_noload = fault_debounce_led1_noload.active;
        fault.led2_noload = fault_debounce_led2_noload.active;
        fault.led3_noload = fault_debounce_led3_noload.active;

        xQueueOverwrite(fault_queue_handle, &fault);
    }
}
