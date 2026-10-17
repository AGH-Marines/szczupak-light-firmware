#include "platform_boot.h"
#include "stm32g0xx_hal.h"

#define PLATFORM_BOOT_REQUEST_MAGIC 0x4F4F4F4FU

void platform_boot_request_set(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTCAPB_CLK_ENABLE();
    TAMP->BKP0R = PLATFORM_BOOT_REQUEST_MAGIC;
    HAL_PWR_DisableBkUpAccess();
}

bool platform_boot_request_get(void) {
    __HAL_RCC_RTCAPB_CLK_ENABLE();
    return TAMP->BKP0R == PLATFORM_BOOT_REQUEST_MAGIC;
}

void platform_boot_request_clear(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTCAPB_CLK_ENABLE();
    TAMP->BKP0R = 0U;
    HAL_PWR_DisableBkUpAccess();
}

void platform_boot_reset(void) {
    NVIC_SystemReset();
}
