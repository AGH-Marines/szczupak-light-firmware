/*
 *  can.h
 *
 *  Created on: 27 Aug 2026
 *  Author: jochman03
 */

#ifndef CAN_H
#define CAN_H

#include "stdint.h"
#include "stdbool.h"

typedef struct can_frame_t {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} can_frame_t;

typedef struct can_command_t {
    uint8_t led1_percent;
    uint8_t led2_percent;
    uint8_t led3_percent;
    bool enable;
    bool fault_reset;
} can_command_t;

typedef struct can_power_status_t {
    uint16_t usb_voltage_mv;
    uint16_t power_voltage_mv;
    uint8_t led1_percent;
    uint8_t led2_percent;
    uint8_t led3_percent;
    bool enable;
} can_power_status_t;

typedef struct can_current_status_t {
    uint16_t led1_current_ma;
    uint16_t led2_current_ma;
    uint16_t led3_current_ma;
} can_current_status_t;

typedef struct can_fault_status_t {
    bool led1_overcurrent;
    bool led2_overcurrent;
    bool led3_overcurrent;
    bool led1_no_load;
    bool led2_no_load;
    bool led3_no_load;
    bool usb_undervoltage;
    bool power_undervoltage;
} can_fault_status_t;

bool can_encode_command(const can_command_t* cmd, can_frame_t* frame);
bool can_decode_command(const can_frame_t* frame, can_command_t* cmd);

bool can_encode_power_status(const can_power_status_t* status, can_frame_t* frame);
bool can_decode_power_status(const can_frame_t* frame, can_power_status_t* status);

bool can_encode_current_status(const can_current_status_t* status, can_frame_t* frame);
bool can_decode_current_status(const can_frame_t* frame, can_current_status_t* status);

bool can_encode_fault_status(const can_fault_status_t* status, can_frame_t* frame);
bool can_decode_fault_status(const can_frame_t* frame, can_fault_status_t* status);

#endif /* CAN_H */
