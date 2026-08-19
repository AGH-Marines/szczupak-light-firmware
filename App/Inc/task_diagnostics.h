/*
 *  task_diagnostics.h
 *
 *  Created on: 18 Aug 2026
 *  Author: jochman03
 */

#ifndef TASK_DIAGNOSTICS_H
#define TASK_DIAGNOSTICS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

typedef struct task_diagnostics_measurement_frame_t {
    uint32_t current_led1;
    uint32_t current_led2;
    uint32_t current_led3;
    uint32_t voltage_12v;
    uint32_t voltage_usb;
} task_diagnostics_measurement_frame_t;

QueueHandle_t task_diagnostics_get_measurement_queue();

void task_diagnostics_init();

#endif /* TASK_DIAGNOSTICS_H */