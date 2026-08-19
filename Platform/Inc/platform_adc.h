/*
 *  platform_adc.h
 *
 *  Created on: 23 Jul 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_ADC_H
#define PLATFORM_ADC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct platform_adc_sample_t {
    uint16_t adc_led1;
    uint16_t adc_led2;
    uint16_t adc_led3;
    uint16_t adc_12v;
    uint16_t adc_usb;
} platform_adc_sample_t;

typedef void (*platform_adc_callback_t)(void);

void platform_adc_init(void);
platform_adc_sample_t platform_adc_get(void);

void platform_adc_set_callback(platform_adc_callback_t callback);

#endif /* PLATFORM_ADC_H */