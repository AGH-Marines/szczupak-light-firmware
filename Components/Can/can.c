#include "can.h"
#include "can_generated.h"

bool can_encode_command(const can_command_t* cmd, can_frame_t* frame) {
    if ((cmd == NULL) || (frame == NULL)) {
        return false;
    }

    if (!can_generated_light_command_led1_setpoint_is_in_range(cmd->led1_percent) ||
        !can_generated_light_command_led2_setpoint_is_in_range(cmd->led2_percent) ||
        !can_generated_light_command_led3_setpoint_is_in_range(cmd->led3_percent)) {
        return false;
    }
    struct can_generated_light_command_t msg = {
        .led1_setpoint = cmd->led1_percent,
        .led2_setpoint = cmd->led2_percent,
        .led3_setpoint = cmd->led3_percent,
        .light_enable = cmd->enable ? 1U : 0U,
    };

    if (can_generated_light_command_pack(frame->data, &msg, sizeof(frame->data)) < 0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_COMMAND_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_COMMAND_LENGTH;
    return true;
}

bool can_decode_command(const can_frame_t* frame, can_command_t* cmd) {
    if ((cmd == NULL) || (frame == NULL)) {
        return false;
    }

    if ((frame->id != CAN_GENERATED_LIGHT_COMMAND_FRAME_ID) ||
        (frame->dlc < CAN_GENERATED_LIGHT_COMMAND_LENGTH)) {
        return false;
    }

    struct can_generated_light_command_t msg;
    if (can_generated_light_command_unpack(&msg, frame->data, frame->dlc) != 0) {
        return false;
    }

    if (!can_generated_light_command_led1_setpoint_is_in_range(msg.led1_setpoint) ||
        !can_generated_light_command_led2_setpoint_is_in_range(msg.led2_setpoint) ||
        !can_generated_light_command_led3_setpoint_is_in_range(msg.led3_setpoint) ||
        !can_generated_light_command_light_enable_is_in_range(msg.light_enable)) {
        return false;
    }

    cmd->led1_percent = msg.led1_setpoint;
    cmd->led2_percent = msg.led2_setpoint;
    cmd->led3_percent = msg.led3_setpoint;
    cmd->enable = (msg.light_enable != 0U);
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

bool can_decode_power_status(const can_frame_t* frame, can_power_status_t* status) {
    if ((status == NULL) || (frame == NULL)) {
        return false;
    }
    if ((frame->id != CAN_GENERATED_LIGHT_POWER_STATUS_FRAME_ID) ||
        (frame->dlc != CAN_GENERATED_LIGHT_POWER_STATUS_LENGTH)) {
        return false;
    }

    struct can_generated_light_power_status_t msg;
    if (can_generated_light_power_status_unpack(&msg, frame->data, frame->dlc) != 0) {
        return false;
    }

    if (!can_generated_light_power_status_usb_voltage_is_in_range(msg.usb_voltage) ||
        !can_generated_light_power_status_power_voltage_is_in_range(msg.power_voltage) ||
        !can_generated_light_power_status_led1_setpoint_is_in_range(msg.led1_setpoint) ||
        !can_generated_light_power_status_led2_setpoint_is_in_range(msg.led2_setpoint) ||
        !can_generated_light_power_status_led3_setpoint_is_in_range(msg.led3_setpoint) ||
        !can_generated_light_power_status_light_enabled_is_in_range(msg.light_enabled)) {
        return false;
    }
    status->usb_voltage_mv = msg.usb_voltage;
    status->power_voltage_mv = msg.power_voltage;
    status->led1_percent = msg.led1_setpoint;
    status->led2_percent = msg.led2_setpoint;
    status->led3_percent = msg.led3_setpoint;
    status->enable = (msg.light_enabled == 0U ? false : true);

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

bool can_decode_current_status(const can_frame_t* frame, can_current_status_t* status) {
    if ((frame == NULL) || (status == NULL)) {
        return false;
    }
    if ((frame->id != CAN_GENERATED_LIGHT_CURRENT_STATUS_FRAME_ID) ||
        (frame->dlc != CAN_GENERATED_LIGHT_CURRENT_STATUS_LENGTH)) {
        return false;
    }

    struct can_generated_light_current_status_t msg;
    if (can_generated_light_current_status_unpack(&msg, frame->data, frame->dlc) != 0) {
        return false;
    }
    if (!can_generated_light_current_status_led1_current_is_in_range(msg.led1_current) ||
        !can_generated_light_current_status_led2_current_is_in_range(msg.led2_current) ||
        !can_generated_light_current_status_led3_current_is_in_range(msg.led3_current)) {
        return false;
    }
    status->led1_current_ma = msg.led1_current;
    status->led2_current_ma = msg.led2_current;
    status->led3_current_ma = msg.led3_current;

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

bool can_decode_fault_status(const can_frame_t* frame, can_fault_status_t* status) {
    if ((frame == NULL) || (status == NULL)) {
        return false;
    }
    if ((frame->id != CAN_GENERATED_LIGHT_FAULT_STATUS_FRAME_ID) ||
        (frame->dlc != CAN_GENERATED_LIGHT_FAULT_STATUS_LENGTH)) {
        return false;
    }

    struct can_generated_light_fault_status_t msg;
    if (can_generated_light_fault_status_unpack(&msg, frame->data, frame->dlc) != 0) {
        return false;
    }

    if (!can_generated_light_fault_status_led1_overcurrent_is_in_range(msg.led1_overcurrent) ||
        !can_generated_light_fault_status_led2_overcurrent_is_in_range(msg.led2_overcurrent) ||
        !can_generated_light_fault_status_led3_overcurrent_is_in_range(msg.led3_overcurrent) ||
        !can_generated_light_fault_status_led1_no_load_is_in_range(msg.led1_no_load) ||
        !can_generated_light_fault_status_led2_no_load_is_in_range(msg.led2_no_load) ||
        !can_generated_light_fault_status_led3_no_load_is_in_range(msg.led3_no_load) ||
        !can_generated_light_fault_status_usb_undervoltage_is_in_range(msg.usb_undervoltage) ||
        !can_generated_light_fault_status_power_undervoltage_is_in_range(msg.power_undervoltage)) {
        return false;
    }

    status->led1_overcurrent = msg.led1_overcurrent;
    status->led2_overcurrent = msg.led2_overcurrent;
    status->led3_overcurrent = msg.led3_overcurrent;
    status->led1_no_load = msg.led1_no_load;
    status->led2_no_load = msg.led2_no_load;
    status->led3_no_load = msg.led3_no_load;
    status->usb_undervoltage = msg.usb_undervoltage;
    status->power_undervoltage = msg.power_undervoltage;

    return true;
}
