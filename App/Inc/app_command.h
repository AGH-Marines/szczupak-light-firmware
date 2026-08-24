/*
 *  app_command.h
 *
 *  Created on: 24 Aug 2026
 *  Author: jochman03
 */

#ifndef APP_COMMAND_H
#define APP_COMMAND_H

typedef enum app_command {
    APP_COMMAND_REQUEST_REGISTERS = 0x10,
    APP_COMMAND_SET_LED = 0x11,
    APP_COMMAND_GET_MEASUREMENTS = 0x12,
    APP_COMMAND_GET_LED = 0x13
} app_command;

#endif /* APP_COMMAND_H */