#include <stddef.h>
#include <string.h>

#include "app_can.h"
#include "app_boot.h"
#include "can.h"

#include "platform_can.h"
#include "platform_boot.h"
#include "platform.h"

#define DEVICE_ID        0U
#define PROTOCOL_VERSION 2U

#define SEGMENT_FRAME_SIZE  7U
#define SEGMENT_FRAME_COUNT 32U
#define SEGMENT_DATA_SIZE   (SEGMENT_FRAME_SIZE * SEGMENT_FRAME_COUNT)

typedef struct app_can_header_t {
    bool part0_received;
    bool part1_received;
    bool accepted;
    uint32_t image_size;
    uint32_t image_crc;
} app_can_header_t;

typedef struct app_can_segment_t {
    uint8_t data[SEGMENT_DATA_SIZE];
    uint32_t received_bitmap;
    uint16_t segment_index;
    bool written;
} app_can_segment_t;

static app_can_header_t app_header;
static app_can_segment_t app_segment;
static bool finish_completed;

static void send_segment_status(uint16_t segment_index, uint32_t segment_bitmap);

void app_can_send_status(can_boot_status_t status) {
    status.device_id = DEVICE_ID;
    can_frame_t frame = {0};

    if (can_encode_boot_status(&status, &frame)) {
        platform_can_transmit(&frame);
    }
}

void app_can_receive() {
    can_frame_t frame = {0};
    if (!platform_can_receive(&frame)) {
        return;
    }

    can_boot_command_t command = {0};
    can_boot_data_t data = {0};
    uint32_t now = platform_get_time_ms();

    if (can_decode_boot_command(&frame, &command)) {
        if ((command.target_mask & (1U << DEVICE_ID)) == 0U) {
            return;
        }
        switch (command.type) {
            case CAN_BOOT_COMMAND_ENTER_UPDATE: {
                if (app_boot_enter_update(now) == APP_BOOT_RESULT_SUCCESS) {
                    app_header = (app_can_header_t){0};
                    app_segment = (app_can_segment_t){0};
                    finish_completed = false;
                    app_can_send_status((can_boot_status_t){.type = CAN_BOOT_STATUS_READY});
                    break;
                }
                break;
            }
            case CAN_BOOT_COMMAND_HEADER: {
                uint8_t command_part = command.payload[0];
                uint8_t command_version = command.payload[1];
                if (command_version != PROTOCOL_VERSION) {
                    if (!app_header.accepted) {
                        app_header = (app_can_header_t){0};
                    }
                    app_can_send_status(
                        (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_INVALID});
                    return;
                }
                uint32_t command_value = 0;
                memcpy(&command_value, command.payload + 2U, sizeof(command_value));

                if (app_header.accepted) {
                    if (command_part == 0U) {
                        if (command_value != app_header.image_size) {
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_INVALID_STATE});
                        }
                        break;
                    } else if (command_part == 1U) {
                        if (command_value != app_header.image_crc) {
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_INVALID_STATE});
                            break;
                        }
                    } else {
                        app_can_send_status(
                            (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_INVALID});
                        break;
                    }
                    app_can_send_status(
                        (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_SUCCESS});
                    break;
                }

                if (command_part == 0U) {
                    app_header.image_size = command_value;
                    app_header.part0_received = true;
                    break;
                } else if (command_part == 1U) {
                    app_header.image_crc = command_value;
                    app_header.part1_received = true;
                } else {
                    app_header = (app_can_header_t){0};
                    app_can_send_status(
                        (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_INVALID});
                    return;
                }
                if (app_header.part0_received && app_header.part1_received) {

                    app_boot_result_t result =
                        app_boot_set_header(app_header.image_size, app_header.image_crc, now);
                    switch (result) {
                        case APP_BOOT_RESULT_SUCCESS: {
                            app_header.accepted = true;
                            app_segment = (app_can_segment_t){0};
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_SUCCESS});

                            break;
                        }
                        case APP_BOOT_RESULT_INVALID_HEADER: {
                            app_header = (app_can_header_t){0};
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_INVALID});

                            break;
                        }
                        case APP_BOOT_RESULT_INVALID_STATE: {
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_INVALID_STATE});

                            break;
                        }
                        case APP_BOOT_RESULT_FLASH_ERROR: {
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_FLASH_ERROR});
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            }

            case CAN_BOOT_COMMAND_SEGMENT_CHECK: {
                uint16_t segment_index;
                memcpy(&segment_index, command.payload, sizeof(segment_index));
                if (app_segment.segment_index != segment_index) {
                    break;
                }

                uint32_t segment_start_offset = app_segment.segment_index * SEGMENT_DATA_SIZE;
                if (segment_start_offset >= app_header.image_size) {
                    app_can_send_status(
                        (can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_INVALID});
                    break;
                }
                uint32_t remaining = app_header.image_size - segment_start_offset;
                uint32_t segment_size =
                    (remaining < SEGMENT_DATA_SIZE) ? remaining : SEGMENT_DATA_SIZE;

                uint8_t required_frames =
                    (segment_size + (SEGMENT_FRAME_SIZE - 1U)) / SEGMENT_FRAME_SIZE;
                uint32_t expected_bitmap = UINT32_MAX;

                if (required_frames < 32U) {
                    expected_bitmap = (1U << required_frames) - 1U;
                }

                if (app_segment.received_bitmap != expected_bitmap) {
                    send_segment_status(segment_index, app_segment.received_bitmap);
                    break;
                }

                if (!app_segment.written) {
                    app_boot_result_t result =
                        app_boot_write(segment_start_offset, app_segment.data, segment_size, now);
                    switch (result) {
                        case APP_BOOT_RESULT_FLASH_ERROR: {
                            app_header = (app_can_header_t){0};
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_FLASH_ERROR});

                            break;
                        }
                        case APP_BOOT_RESULT_SUCCESS: {
                            app_segment.written = true;
                            send_segment_status(segment_index, app_segment.received_bitmap);
                            break;
                        }
                        case APP_BOOT_RESULT_INVALID_STATE: {
                            app_can_send_status(
                                (can_boot_status_t){.type = CAN_BOOT_STATUS_INVALID_STATE});
                            break;
                        }
                        case APP_BOOT_RESULT_INVALID_ARGUMENT: {
                            break;
                        }
                        default:
                            break;
                    }
                } else {
                    send_segment_status(segment_index, app_segment.received_bitmap);
                }
                break;
            }

            case CAN_BOOT_COMMAND_FINISH: {
                if (finish_completed) {
                    app_can_send_status(
                        (can_boot_status_t){.type = CAN_BOOT_STATUS_FINISH_SUCCESS});
                    break;
                }
                app_boot_result_t result = app_boot_finish(now);
                switch (result) {
                    case APP_BOOT_RESULT_SUCCESS: {
                        finish_completed = true;
                        app_can_send_status(
                            (can_boot_status_t){.type = CAN_BOOT_STATUS_FINISH_SUCCESS});

                        break;
                    }
                    case APP_BOOT_RESULT_CRC_MISMATCH: {
                        app_header = (app_can_header_t){0};
                        app_can_send_status(
                            (can_boot_status_t){.type = CAN_BOOT_STATUS_CRC_MISMATCH});

                        break;
                    }
                    case APP_BOOT_RESULT_FLASH_ERROR: {
                        app_header = (app_can_header_t){0};
                        app_can_send_status(
                            (can_boot_status_t){.type = CAN_BOOT_STATUS_FLASH_ERROR});

                        break;
                    }
                    case APP_BOOT_RESULT_INVALID_STATE: {
                        app_can_send_status(
                            (can_boot_status_t){.type = CAN_BOOT_STATUS_INVALID_STATE});
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            case CAN_BOOT_COMMAND_RESET: {
                platform_boot_reset();

                break;
            }
            default:
                break;
        }
    } else if (can_decode_boot_data(&frame, &data)) {
        if (data.frame_index >= SEGMENT_FRAME_COUNT) {
            return;
        }
        if (app_segment.written) {
            app_segment.segment_index++;
            app_segment.received_bitmap = 0U;
            app_segment.written = false;
        }
        uint32_t offset = data.frame_index * SEGMENT_FRAME_SIZE;
        memcpy(app_segment.data + offset, data.payload, SEGMENT_FRAME_SIZE);
        app_segment.received_bitmap |= (1U << data.frame_index);
    }
}

static void send_segment_status(uint16_t segment_index, uint32_t segment_bitmap) {
    can_boot_status_t status = {.type = CAN_BOOT_STATUS_SEGMENT};

    memcpy(&status.payload[0], &segment_index, sizeof(segment_index));
    memcpy(&status.payload[2], &segment_bitmap, sizeof(segment_bitmap));

    app_can_send_status(status);
}