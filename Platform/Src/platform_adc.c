
#include <stddef.h>

#include "platform_adc.h"
#include "adc.h"
#include "tim.h"

#define PLATFORM_ADC_CHANNEL_COUNT      5U
#define PLATFORM_ADC_SEQUENCES_COUNT    16U
#define PLATFORM_ADC_BUFFER_HALF_LENGTH (PLATFORM_ADC_CHANNEL_COUNT * PLATFORM_ADC_SEQUENCES_COUNT)
#define PLATFORM_ADC_BUFFER_LENGTH      (PLATFORM_ADC_BUFFER_HALF_LENGTH * 2U)

static ADC_HandleTypeDef* platform_adc = &hadc1;
static TIM_HandleTypeDef* platform_adc_trigger = &htim6;

static platform_adc_sample_t latest_sample;
static uint16_t platform_adc_buffer[PLATFORM_ADC_BUFFER_LENGTH];

static void process_data(size_t index);
static platform_adc_callback_t adc_ready_callback;

void platform_adc_init(void) {
    HAL_ADCEx_Calibration_Start(platform_adc);

    HAL_ADC_Start_DMA(platform_adc, (uint32_t*)platform_adc_buffer, PLATFORM_ADC_BUFFER_LENGTH);
    HAL_TIM_Base_Start(platform_adc_trigger);
}

platform_adc_sample_t platform_adc_get(void) {
    return latest_sample;
}

void platform_adc_set_callback(platform_adc_callback_t callback) {
    adc_ready_callback = callback;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc != platform_adc) {
        return;
    }
    process_data(0U);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc != platform_adc) {
        return;
    }
    process_data(PLATFORM_ADC_BUFFER_HALF_LENGTH);
}

static void process_data(size_t index) {
    uint32_t current_sample[PLATFORM_ADC_CHANNEL_COUNT] = {0};
    for (size_t channel_number = 0U; channel_number < PLATFORM_ADC_CHANNEL_COUNT;
         ++channel_number) {
        for (size_t sequence_number = 0U; sequence_number < PLATFORM_ADC_SEQUENCES_COUNT;
             ++sequence_number) {
            size_t buffer_index =
                index + ((sequence_number * PLATFORM_ADC_CHANNEL_COUNT) + channel_number);
            current_sample[channel_number] += platform_adc_buffer[buffer_index];
        }
    }
    for (size_t channel_number = 0U; channel_number < PLATFORM_ADC_CHANNEL_COUNT;
         ++channel_number) {
        current_sample[channel_number] /= PLATFORM_ADC_SEQUENCES_COUNT;
    }
    latest_sample.adc_led1 = (uint16_t)current_sample[0];
    latest_sample.adc_led2 = (uint16_t)current_sample[1];
    latest_sample.adc_led3 = (uint16_t)current_sample[2];
    latest_sample.adc_12v = (uint16_t)current_sample[3];
    latest_sample.adc_usb = (uint16_t)current_sample[4];

    if (adc_ready_callback != NULL) {
        adc_ready_callback();
    }
}
