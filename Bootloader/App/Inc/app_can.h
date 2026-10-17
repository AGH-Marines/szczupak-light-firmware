/*
 *  app_can.h
 *
 *  Created on: 02 Oct 2026
 *  Author: jochman03
 */

#ifndef APP_CAN_H
#define APP_CAN_H

#include <stdint.h>
#include <can.h>

void app_can_send_status(can_boot_status_t status);

void app_can_receive();

#endif /* APP_CAN_H */