/*
 *  platform_reset.h
 *
 *  Created on: 24 Aug 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_RESET_H
#define PLATFORM_RESET_H

typedef enum platform_reset_source {
    PLATFORM_RESET_UNKNOWN,
    PLATFORM_RESET_PIN,
    PLATFORM_RESET_POR_BOR,
    PLATFORM_RESET_SOFTWARE,
    PLATFORM_RESET_IWDG,
    PLATFORM_RESET_WWDG,
    PLATFORM_RESET_OPTION_BYTE,
    PLATFORM_RESET_LPWR
} platform_reset_source_t;

platform_reset_source_t platform_reset_get_source(void);
const char* platform_reset_source(platform_reset_source_t source);
const char* platform_reset_reason(void);

#endif /* PLATFORM_RESET_H */