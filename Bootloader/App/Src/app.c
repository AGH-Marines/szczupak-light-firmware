#include "app.h"
#include "platform_led.h"
#include "platform_memory.h"
#include "platform.h"

void app_init(void) {
    for (uint8_t i = 0; i < 3; ++i) {
        platform_led_status_toggle();
        platform_delay(200);
    }

    if (platform_memory_app_valid()) {
        platform_memory_app_start();
    }
}

void app_update(void) {}