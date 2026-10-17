#include "can.h"
#include "can_generated.h"

bool can_decode_boot_command(const can_frame_t* frame, can_boot_command_t* command) {
    if ((frame == NULL) || (command == NULL)) {
        return false;
    }

    if ((frame->id != CAN_GENERATED_LIGHT_BOOT_COMMAND_FRAME_ID) ||
        frame->dlc != CAN_GENERATED_LIGHT_BOOT_COMMAND_LENGTH) {
        return false;
    }

    struct can_generated_light_boot_command_t generated_cmd;

    if (can_generated_light_boot_command_unpack(&generated_cmd, frame->data, frame->dlc) != 0) {
        return false;
    }

    if (!can_generated_light_boot_command_type_is_in_range(generated_cmd.type)) {
        return false;
    }

    command->type = generated_cmd.type;
    command->target_mask = generated_cmd.target_mask;
    command->payload[0] = generated_cmd.payload0;
    command->payload[1] = generated_cmd.payload1;
    command->payload[2] = generated_cmd.payload2;
    command->payload[3] = generated_cmd.payload3;
    command->payload[4] = generated_cmd.payload4;
    command->payload[5] = generated_cmd.payload5;

    return true;
}

bool can_decode_boot_data(const can_frame_t* frame, can_boot_data_t* data) {
    if ((frame == NULL) || (data == NULL)) {
        return false;
    }

    if ((frame->id != CAN_GENERATED_LIGHT_BOOT_DATA_FRAME_ID) ||
        (frame->dlc != CAN_GENERATED_LIGHT_BOOT_DATA_LENGTH)) {
        return false;
    }

    struct can_generated_light_boot_data_t generated_data;

    if (can_generated_light_boot_data_unpack(&generated_data, frame->data, frame->dlc) != 0) {
        return false;
    }
    if (!can_generated_light_boot_data_frame_index_is_in_range(generated_data.frame_index)) {
        return false;
    }

    data->frame_index = generated_data.frame_index;
    data->payload[0] = generated_data.payload0;
    data->payload[1] = generated_data.payload1;
    data->payload[2] = generated_data.payload2;
    data->payload[3] = generated_data.payload3;
    data->payload[4] = generated_data.payload4;
    data->payload[5] = generated_data.payload5;
    data->payload[6] = generated_data.payload6;

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
    generated_status.payload0 = status->payload[0];
    generated_status.payload1 = status->payload[1];
    generated_status.payload2 = status->payload[2];
    generated_status.payload3 = status->payload[3];
    generated_status.payload4 = status->payload[4];
    generated_status.payload5 = status->payload[5];

    if (can_generated_light_boot_status_pack(frame->data, &generated_status, sizeof(frame->data)) <
        0) {
        return false;
    }

    frame->id = CAN_GENERATED_LIGHT_BOOT_STATUS_FRAME_ID;
    frame->dlc = CAN_GENERATED_LIGHT_BOOT_STATUS_LENGTH;

    return true;
}
