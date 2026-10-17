/*
 *  app_boot.h
 *
 *  Created on: 01 Oct 2026
 *  Author: jochman03
 */

#ifndef APP_BOOT_H
#define APP_BOOT_H

#include <stdint.h>

#define APP_BOOT_INITIAL_WAIT_MS 1000U
#define APP_BOOT_STATE_WAIT_MS   20000U

typedef enum app_boot_state_t {
    APP_BOOT_WAIT_UPDATE = 0,
    APP_BOOT_WAIT_HEADER,
    APP_BOOT_WAIT_IMAGE,
    APP_BOOT_WAIT_RESET,
    APP_BOOT_APP_START,
} app_boot_state_t;

typedef enum app_boot_result_t {
    APP_BOOT_RESULT_SUCCESS = 0,
    APP_BOOT_RESULT_INVALID_STATE,
    APP_BOOT_RESULT_INVALID_HEADER,
    APP_BOOT_RESULT_INVALID_ARGUMENT,
    APP_BOOT_RESULT_FLASH_ERROR,
    APP_BOOT_RESULT_CRC_MISMATCH,
} app_boot_result_t;

typedef enum boot_state_event_t {
    APP_BOOT_EVENT_NONE = 0,
    APP_BOOT_EVENT_HEADER_TIMEOUT,
    APP_BOOT_EVENT_SEGMENT_TIMEOUT,
    APP_BOOT_EVENT_APP_START,
} boot_state_event_t;

typedef struct app_boot_header_t {
    uint32_t image_size;
    uint32_t image_crc;
} app_boot_header_t;

boot_state_event_t app_boot_update(uint32_t now);

app_boot_result_t app_boot_init(uint32_t now);
app_boot_result_t app_boot_enter_update(uint32_t now);
app_boot_result_t app_boot_set_header(uint32_t image_size, uint32_t image_crc, uint32_t now);
app_boot_result_t app_boot_write(uint32_t offset, const uint8_t* data, uint32_t size, uint32_t now);
app_boot_result_t app_boot_finish(uint32_t now);

#endif /* APP_BOOT_H */