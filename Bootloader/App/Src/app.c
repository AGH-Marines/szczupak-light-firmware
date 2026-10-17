#include <string.h>

#include "app.h"
#include "can.h"
#include "app_can.h"
#include "app_boot.h"

#include "platform_led.h"
#include "platform_memory.h"
#include "platform_boot.h"
#include "platform_can.h"
#include "platform.h"

#define APP_START_WAIT_MS 100U

static bool app_start_pending;
static uint32_t app_start_time;

void app_init(void) {
    platform_can_init();
    uint32_t now = platform_get_time_ms();
    app_boot_init(now);
    app_start_pending = false;

    if (platform_boot_request_get()) {
        platform_boot_request_clear();
        if (app_boot_enter_update(now) == APP_BOOT_RESULT_SUCCESS) {
            app_can_send_status((can_boot_status_t){.type = CAN_BOOT_STATUS_READY});
        }
    }
}

void app_update(void) {

    app_can_receive();

    uint32_t now = platform_get_time_ms();
    boot_state_event_t event = app_boot_update(now);

    switch (event) {
        case APP_BOOT_EVENT_APP_START: {
            if (!app_start_pending) {
                app_start_time = now;
                app_start_pending = true;
            }
            if (now - app_start_time < APP_START_WAIT_MS) {
                break;
            }
            if (platform_can_tx_fifo_empty()) {
                app_start_pending = false;
                platform_memory_app_start();
            }
            break;
        }
        case APP_BOOT_EVENT_HEADER_TIMEOUT: {
            app_can_send_status((can_boot_status_t){.type = CAN_BOOT_STATUS_HEADER_TIMEOUT});
            break;
        }
        case APP_BOOT_EVENT_SEGMENT_TIMEOUT: {
            app_can_send_status((can_boot_status_t){.type = CAN_BOOT_STATUS_SEGMENT_TIMEOUT});
            break;
        }
        default:
            break;
    }
}