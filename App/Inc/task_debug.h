/*
 *  task_debug.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef TASK_DEBUG_H
#define TASK_DEBUG_H

#include "debugger.h"

#if DEBUGGER_ENABLED

typedef enum app_command { APP_COMMAND_REQUEST_REGISTERS = 1, APP_COMMAND_USER } app_command;

typedef enum signal {
    SIGNAL_COUNTER8,
    SIGNAL_LED1,
    SIGNAL_LED2,
    SIGNAL_LED3,
    SIGNAL_12V,
    SIGNAL_USB
} signal;

void task_debug_init();

#endif /* DEBUGGER_ENABLED */

#endif /* TASK_DEBUG_H */