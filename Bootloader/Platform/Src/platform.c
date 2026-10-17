#include <stdint.h>
#include "main.h"
#include "platform.h"

void platform_delay(uint32_t delay) {
    HAL_Delay(delay);
}

uint32_t platform_get_time_ms() {
    return HAL_GetTick();
}
