/*
 *  app_faults.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef APP_FAULTS_H
#define APP_FAULTS_H

typedef enum app_fault_type_t {
    APP_FAULT_UNKNOWN = 1,
    APP_FAULT_HARDFAULT,
    APP_FAULT_NMI,
    APP_FAULT_ASSERT,
    APP_FAULT_MALLOC,
    APP_FAULT_STACK_OVERFLOW,
    APP_FAULT_HAL,
    APP_FAULT_UNHANDLED_IRQ,
    APP_FAULT_WATCHDOG,
} app_fault_type_t;

void app_faults_fatal(app_fault_type_t type);
void app_faults_assert(void);

#endif /* APP_FAULTS_H */