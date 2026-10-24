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

typedef enum can_boot_command_type_t {
    CAN_BOOT_COMMAND_INVALID = 0,
    CAN_BOOT_COMMAND_ENTER_UPDATE,
    CAN_BOOT_COMMAND_HEADER,
    CAN_BOOT_COMMAND_SEGMENT_CHECK,
    CAN_BOOT_COMMAND_FINISH,
    CAN_BOOT_COMMAND_RESET,
    CAN_BOOT_COMMAND_STATUS_REQUEST,
} can_boot_command_type_t;

typedef struct can_boot_command_t {
    can_boot_command_type_t type;
    uint8_t target_mask;
    uint8_t header_part;
    uint8_t protocol_version;
    uint32_t header_value;
    uint16_t segment_index;
} can_boot_command_t;

typedef struct can_boot_data_t {
    uint8_t frame_index;
    uint8_t payload[7];
} can_boot_data_t;

typedef enum can_boot_status_type_t {
    CAN_BOOT_STATUS_INVALID = 0,
    CAN_BOOT_STATUS_READY,
    CAN_BOOT_STATUS_HEADER_SUCCESS,
    CAN_BOOT_STATUS_HEADER_INVALID,
    CAN_BOOT_STATUS_HEADER_TIMEOUT,
    CAN_BOOT_STATUS_SEGMENT,
    CAN_BOOT_STATUS_SEGMENT_TIMEOUT,
    CAN_BOOT_STATUS_FINISH_SUCCESS,
    CAN_BOOT_STATUS_CRC_MISMATCH,
    CAN_BOOT_STATUS_FLASH_ERROR,
    CAN_BOOT_STATUS_INVALID_STATE,
    CAN_BOOT_STATUS_BOOTLOADER,
    CAN_BOOT_STATUS_APP_READY,
    CAN_BOOT_STATUS_APP_ERROR,
} can_boot_status_type_t;

typedef struct can_boot_status_t {
    uint8_t device_id;
    can_boot_status_type_t type;

    uint16_t segment_index;
    uint32_t received_bitmap;
    uint32_t error_code;
} can_boot_status_t;

bool can_decode_boot_command(const can_frame_t* frame, can_boot_command_t* command);
bool can_decode_boot_data(const can_frame_t* frame, can_boot_data_t* data);
bool can_encode_boot_status(const can_boot_status_t* status, can_frame_t* frame);

#endif /* CAN_H */
