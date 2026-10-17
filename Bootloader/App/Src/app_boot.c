#include <stddef.h>

#include "app_boot.h"

#include "platform_memory.h"
#include "platform_boot.h"
#include "platform_led.h"

#define LED_INIT_PERIOD_MS   300U
#define LED_HEADER_PERIOD_MS 100U
#define LED_IMAGE_PERIOD_MS  50U

static uint32_t state_update_time;
static uint32_t led_update_time;

static app_boot_state_t boot_state = APP_BOOT_WAIT_UPDATE;
static app_boot_header_t header;

app_boot_result_t app_boot_init(uint32_t now) {
    header = (app_boot_header_t){0};
    state_update_time = now;
    led_update_time = now;
    boot_state = APP_BOOT_WAIT_UPDATE;

    platform_led_status_set(false);
    platform_led_fault_set(false);

    return APP_BOOT_RESULT_SUCCESS;
}

app_boot_result_t app_boot_enter_update(uint32_t now) {
    if (boot_state == APP_BOOT_WAIT_HEADER) {
        header = (app_boot_header_t){0};
        state_update_time = now;
        return APP_BOOT_RESULT_SUCCESS;
    }
    if (boot_state != APP_BOOT_WAIT_UPDATE) {
        return APP_BOOT_RESULT_INVALID_STATE;
    }
    state_update_time = now;
    boot_state = APP_BOOT_WAIT_HEADER;
    header = (app_boot_header_t){0};
    return APP_BOOT_RESULT_SUCCESS;
}

boot_state_event_t app_boot_update(uint32_t now) {
    switch (boot_state) {
        case APP_BOOT_WAIT_UPDATE: {
            if (now - led_update_time > LED_INIT_PERIOD_MS) {
                led_update_time = now;
                platform_led_fault_toggle();
                platform_led_status_toggle();
            }
            if (now - state_update_time > APP_BOOT_INITIAL_WAIT_MS) {
                state_update_time = now;
                boot_state = APP_BOOT_APP_START;
            }
            if (!platform_memory_app_valid()) {
                break;
            }
            break;
        }
        case APP_BOOT_WAIT_HEADER: {
            if (now - led_update_time > LED_HEADER_PERIOD_MS) {
                led_update_time = now;
                platform_led_fault_toggle();
                platform_led_status_toggle();
            }
            if (now - state_update_time > APP_BOOT_STATE_WAIT_MS) {
                state_update_time = now;
                boot_state = APP_BOOT_WAIT_UPDATE;
                return APP_BOOT_EVENT_HEADER_TIMEOUT;
            }
            break;
        }
        case APP_BOOT_WAIT_IMAGE: {
            if (now - led_update_time > LED_IMAGE_PERIOD_MS) {
                led_update_time = now;
                platform_led_fault_toggle();
                platform_led_status_toggle();
            }
            if (now - state_update_time > APP_BOOT_STATE_WAIT_MS) {
                state_update_time = now;
                boot_state = APP_BOOT_WAIT_UPDATE;
                return APP_BOOT_EVENT_SEGMENT_TIMEOUT;
            }
            break;
        }
        case APP_BOOT_WAIT_RESET: {
            break;
        }
        case APP_BOOT_APP_START: {
            if (platform_memory_app_valid()) {
                platform_led_status_set(false);
                platform_led_fault_set(false);
                return APP_BOOT_EVENT_APP_START;
            }
            state_update_time = now;
            boot_state = APP_BOOT_WAIT_UPDATE;
            break;
        }
        default:
            break;
    }
    return APP_BOOT_EVENT_NONE;
}

app_boot_result_t app_boot_set_header(uint32_t image_size, uint32_t image_crc, uint32_t now) {
    if (boot_state != APP_BOOT_WAIT_HEADER) {
        return APP_BOOT_RESULT_INVALID_STATE;
    }
    if (image_size == 0 || image_size > platform_memory_app_size()) {
        return APP_BOOT_RESULT_INVALID_HEADER;
    }
    if (!platform_memory_metadata_clear()) {
        return APP_BOOT_RESULT_FLASH_ERROR;
    }
    if (!platform_memory_app_clear(image_size)) {
        return APP_BOOT_RESULT_FLASH_ERROR;
    }
    header.image_size = image_size;
    header.image_crc = image_crc;

    state_update_time = now;
    boot_state = APP_BOOT_WAIT_IMAGE;

    return APP_BOOT_RESULT_SUCCESS;
}

app_boot_result_t app_boot_write(uint32_t offset, const uint8_t* data, uint32_t size,
                                 uint32_t now) {
    if (boot_state != APP_BOOT_WAIT_IMAGE) {
        return APP_BOOT_RESULT_INVALID_STATE;
    }
    if (data == NULL || size == 0U || offset >= header.image_size) {
        return APP_BOOT_RESULT_INVALID_ARGUMENT;
    }

    if (size > header.image_size - offset) {
        return APP_BOOT_RESULT_INVALID_ARGUMENT;
    }

    if (!platform_memory_app_write(offset, data, size)) {
        header = (app_boot_header_t){0};
        state_update_time = now;
        boot_state = APP_BOOT_WAIT_HEADER;
        return APP_BOOT_RESULT_FLASH_ERROR;
    }
    state_update_time = now;

    return APP_BOOT_RESULT_SUCCESS;
}

app_boot_result_t app_boot_finish(uint32_t now) {
    if (boot_state != APP_BOOT_WAIT_IMAGE) {
        return APP_BOOT_RESULT_INVALID_STATE;
    }
    uint32_t app_crc = 0U;

    if (!platform_memory_app_calculate_crc(header.image_size, &app_crc)) {
        header = (app_boot_header_t){0};
        state_update_time = now;
        boot_state = APP_BOOT_WAIT_HEADER;
        return APP_BOOT_RESULT_FLASH_ERROR;
    }
    if (header.image_crc != app_crc) {
        header = (app_boot_header_t){0};
        state_update_time = now;
        boot_state = APP_BOOT_WAIT_HEADER;
        return APP_BOOT_RESULT_CRC_MISMATCH;
    }
    if (!platform_memory_metadata_save(header.image_size, header.image_crc)) {
        header = (app_boot_header_t){0};
        state_update_time = now;
        boot_state = APP_BOOT_WAIT_HEADER;
        return APP_BOOT_RESULT_FLASH_ERROR;
    }
    state_update_time = now;
    boot_state = APP_BOOT_WAIT_RESET;
    return APP_BOOT_RESULT_SUCCESS;
}