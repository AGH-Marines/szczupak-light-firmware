/*
 *  task_debug.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef TASK_DEBUG_H
#define TASK_DEBUG_H

#include "debugger.h"
#include "app_command.h"

#if DEBUGGER_ENABLED

typedef enum signal {
    SIGNAL_COUNTER8,
    SIGNAL_LED1,
    SIGNAL_LED2,
    SIGNAL_LED3,
    SIGNAL_12V,
    SIGNAL_USB,
    SIGNAL_PWM_LED1,
    SIGNAL_PWM_LED2,
    SIGNAL_PWM_LED3
} signal;

void task_debug_init();

#endif /* DEBUGGER_ENABLED */

#endif /* TASK_DEBUG_H */