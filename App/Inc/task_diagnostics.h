/*
 *  task_diagnostics.h
 *
 *  Created on: 18 Aug 2026
 *  Author: jochman03
 */

#ifndef TASK_DIAGNOSTICS_H
#define TASK_DIAGNOSTICS_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"

typedef struct task_diagnostics_measurement_frame_t {
    uint32_t current_led1;
    uint32_t current_led2;
    uint32_t current_led3;
    uint32_t voltage_12v;
    uint32_t voltage_usb;
} task_diagnostics_measurement_frame_t;

typedef struct task_diagnostics_fault_frame_t {
    bool led1_overcurrent;
    bool led2_overcurrent;
    bool led3_overcurrent;
    bool led1_noload;
    bool led2_noload;
    bool led3_noload;
    bool usb_undervoltage;
    bool power_undervoltage;
} task_diagnostics_fault_frame_t;

QueueHandle_t task_diagnostics_get_measurement_queue();
QueueHandle_t task_diagnostics_get_fault_queue();

void task_diagnostics_init();
void task_diagnostics_clear_faults();

#endif /* TASK_DIAGNOSTICS_H */