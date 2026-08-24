#include <stdio.h>

#include "platform_reset.h"
#include "main.h"

platform_reset_source_t platform_reset_get_source(void) {
    const uint32_t flags = HAL_RCC_GetResetSource();
    if ((flags & RCC_RESET_FLAG_LPWR) != 0U) {
        return PLATFORM_RESET_LPWR;
    }
    if ((flags & RCC_RESET_FLAG_WWDG) != 0U) {
        return PLATFORM_RESET_WWDG;
    }
    if ((flags & RCC_RESET_FLAG_IWDG) != 0U) {
        return PLATFORM_RESET_IWDG;
    }
    if ((flags & RCC_RESET_FLAG_SW) != 0U) {
        return PLATFORM_RESET_SOFTWARE;
    }
    if ((flags & RCC_RESET_FLAG_OBL) != 0U) {
        return PLATFORM_RESET_OPTION_BYTE;
    }
    if ((flags & RCC_RESET_FLAG_PWR) != 0U) {
        return PLATFORM_RESET_POR_BOR;
    }
    if ((flags & RCC_RESET_FLAG_PIN) != 0U) {
        return PLATFORM_RESET_PIN;
    }
    return PLATFORM_RESET_UNKNOWN;
}

const char* platform_reset_source(platform_reset_source_t source) {
    switch (source) {
        case PLATFORM_RESET_PIN:
            return "pin";
        case PLATFORM_RESET_POR_BOR:
            return "por-bor";
        case PLATFORM_RESET_SOFTWARE:
            return "software";
        case PLATFORM_RESET_IWDG:
            return "iwdg";
        case PLATFORM_RESET_WWDG:
            return "wwdg";
        case PLATFORM_RESET_OPTION_BYTE:
            return "option-byte";
        case PLATFORM_RESET_LPWR:
            return "lpwr";
        default:
            return "unknown";
    }
}

const char* platform_reset_reason(void) {
    static char reason[24];
    const char* source = platform_reset_source(platform_reset_get_source());
    (void)snprintf(reason, sizeof(reason), "reason: %s", source);
    return reason;
}