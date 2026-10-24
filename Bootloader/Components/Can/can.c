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
