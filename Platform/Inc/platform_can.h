/*
 *  platform_can.h
 *
 *  Created on: 29 Aug 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_CAN_H
#define PLATFORM_CAN_H

#include "can.h"

typedef void (*platform_can_receive_callback_t)(const can_frame_t* frame);
typedef void (*platform_can_reset_callback_t)(void);

void platform_can_init(void);
bool platform_can_transmit(const can_frame_t* frame);
void platform_can_set_receive_callback(platform_can_receive_callback_t cb);
void platform_can_set_reset_callback(platform_can_reset_callback_t cb);

void platform_can_reset(void);

#endif /* PLATFORM_CAN_H */