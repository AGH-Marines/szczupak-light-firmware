#include "can.h"
#include "can_generated.h"

bool can_decode_command(const can_frame_t* frame, can_command_t* cmd) {
    if ((cmd == NULL) || (frame == NULL)) {
        return false;
    }

    if ((frame->id != CAN_GENERATED_LIGHT_COMMAND_FRAME_ID) ||
        (frame->dlc < CAN_GENERATED_LIGHT_COMMAND_LENGTH)) {
        return false;
    }

    struct can_generated_light_command_t msg;
    if (can_generated_light_command_unpack(&msg, frame->data, frame->dlc) != 0U) {
        return false;
    }

    if (!can_generated_light_command_led1_setpoint_is_in_range(msg.led1_setpoint) ||
        !can_generated_light_command_led2_setpoint_is_in_range(msg.led2_setpoint) ||
        !can_generated_light_command_led3_setpoint_is_in_range(msg.led3_setpoint) ||
        !can_generated_light_command_light_enable_is_in_range(msg.light_enable) ||
        !can_generated_light_command_fault_reset_is_in_range(msg.fault_reset)) {
        return false;
    }

    cmd->led1_percent = msg.led1_setpoint;
    cmd->led2_percent = msg.led2_setpoint;
    cmd->led3_percent = msg.led3_setpoint;
    cmd->enable = (msg.light_enable != 0U);
    cmd->fault_reset = (msg.fault_reset != 0U);
    return true;
}

bool can_encode_power_status(const can_power_status_t* status, can_frame_t* frame) {
    if ((status == NULL) || (frame == NULL)) {
        return false;
    }
    if (!can_generated_light_power_status_usb_voltage_is_in_range(status->usb_voltage_mv) ||
        !can_generated_light_power_status_power_voltage_is_in_range(status->power_voltage_mv) ||
        !can_generated_light_power_status_led1_setpoint_is_in_range(status->led1_percent) ||
        !can_generated_light_power_status_led1_setpoint_is_in_range(status->led2_percent) ||
        !can_generated_light_power_status_led1_setpoint_is_in_range(status->led3_percent) ||
        !can_generated_light_power_status_light_enabled_is_in_range(status->enable)) {
        return false;
    }

    struct can_generated_light_power_status_t msg = {
        .usb_voltage = status->usb_voltage_mv,
        .power_voltage = status->power_voltage_mv,
        .led1_setpoint = status->led1_percent,
        .led2_setpoint = status->led2_percent,
        .led3_setpoint = status->led3_percent,
        .light_enabled = status->enable ? 1U : 0U,
    };

    if (can_generated_light_power_status_pack(frame->data, &msg, sizeof(frame->data)) < 0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_POWER_STATUS_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_POWER_STATUS_LENGTH;
    return true;
}

bool can_encode_current_status(const can_current_status_t* status, can_frame_t* frame) {
    if ((status == NULL) || (frame == NULL)) {
        return false;
    }
    if (!can_generated_light_current_status_led1_current_is_in_range(status->led1_current_ma) ||
        !can_generated_light_current_status_led2_current_is_in_range(status->led2_current_ma) ||
        !can_generated_light_current_status_led3_current_is_in_range(status->led3_current_ma)) {
        return false;
    }
    struct can_generated_light_current_status_t msg = {
        .led1_current = status->led1_current_ma,
        .led2_current = status->led2_current_ma,
        .led3_current = status->led3_current_ma,
    };

    if (can_generated_light_current_status_pack(frame->data, &msg, sizeof(frame->data)) < 0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_CURRENT_STATUS_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_CURRENT_STATUS_LENGTH;
    return true;
}

bool can_encode_fault_status(const can_fault_status_t* status, can_frame_t* frame) {
    if ((status == NULL) || (frame == NULL)) {
        return false;
    }
    if (!can_generated_light_fault_status_led1_overcurrent_is_in_range(status->led1_overcurrent) ||
        !can_generated_light_fault_status_led2_overcurrent_is_in_range(status->led2_overcurrent) ||
        !can_generated_light_fault_status_led3_overcurrent_is_in_range(status->led3_overcurrent) ||
        !can_generated_light_fault_status_led1_no_load_is_in_range(status->led1_no_load) ||
        !can_generated_light_fault_status_led2_no_load_is_in_range(status->led2_no_load) ||
        !can_generated_light_fault_status_led3_no_load_is_in_range(status->led3_no_load) ||
        !can_generated_light_fault_status_usb_undervoltage_is_in_range(status->usb_undervoltage) ||
        !can_generated_light_fault_status_power_undervoltage_is_in_range(
            status->power_undervoltage)) {
        return false;
    }

    struct can_generated_light_fault_status_t msg = {
        .led1_overcurrent = status->led1_overcurrent,
        .led2_overcurrent = status->led2_overcurrent,
        .led3_overcurrent = status->led3_overcurrent,
        .led1_no_load = status->led1_no_load,
        .led2_no_load = status->led2_no_load,
        .led3_no_load = status->led3_no_load,
        .usb_undervoltage = status->usb_undervoltage,
        .power_undervoltage = status->power_undervoltage,
    };

    if (can_generated_light_fault_status_pack(frame->data, &msg, sizeof(frame->data)) < 0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_FAULT_STATUS_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_FAULT_STATUS_LENGTH;
    return true;
}

bool can_decode_boot_command(const can_frame_t* frame, can_boot_command_t* command) {
    if ((frame == NULL) || (command == NULL)) {
        return false;
    }

    if ((frame->id != CAN_GENERATED_LIGHT_BOOT_COMMAND_FRAME_ID) ||
        frame->dlc != CAN_GENERATED_LIGHT_BOOT_COMMAND_LENGTH) {
        return false;
    }

    struct can_generated_light_boot_command_t generated_cmd = {0};

    if (can_generated_light_boot_command_unpack(&generated_cmd, frame->data, frame->dlc) != 0) {
        return false;
    }

    if ((generated_cmd.type <= CAN_BOOT_COMMAND_INVALID) ||
        !can_generated_light_boot_command_type_is_in_range(generated_cmd.type)) {
        return false;
    }

    *command = (can_boot_command_t){0};
    command->type = generated_cmd.type;
    command->target_mask = generated_cmd.target_mask;
    command->header_part = generated_cmd.header_part;
    command->protocol_version = generated_cmd.protocol_version;
    command->header_value = generated_cmd.header_value;
    command->segment_index = generated_cmd.segment_index;

    return true;
}

bool can_encode_boot_status(const can_boot_status_t* status, can_frame_t* frame) {
    if ((status == NULL) || (frame == NULL)) {
        return false;
    }

    if (!can_generated_light_boot_status_device_id_is_in_range(status->device_id)) {
        return false;
    }

    if ((status->type <= CAN_BOOT_STATUS_INVALID) ||
        !can_generated_light_boot_status_type_is_in_range(status->type)) {
        return false;
    }

    struct can_generated_light_boot_status_t generated_status = {0};

    generated_status.device_id = status->device_id;
    generated_status.type = status->type;
    generated_status.error_code = status->error_code;
    generated_status.received_bitmap = status->received_bitmap;
    generated_status.segment_index = status->segment_index;

    if (can_generated_light_boot_status_pack(frame->data, &generated_status, sizeof(frame->data)) <
        0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_BOOT_STATUS_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_BOOT_STATUS_LENGTH;

    return true;
}
