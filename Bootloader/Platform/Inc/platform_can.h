/*
 *  platform_can.h
 *
 *  Created on: 21 Sep 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_CAN_H
#define PLATFORM_CAN_H

#include "can.h"

void platform_can_init(void);
bool platform_can_transmit(const can_frame_t* frame);
bool platform_can_receive(can_frame_t* frame);
bool platform_can_tx_fifo_empty(void);

#endif /* PLATFORM_CAN_H */