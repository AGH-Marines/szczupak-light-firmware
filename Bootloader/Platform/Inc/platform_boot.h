/*
 *  platform_boot.h
 *
 *  Created on: 21 Sep 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_BOOT_H
#define PLATFORM_BOOT_H

#include <stdbool.h>

void platform_boot_request_set(void);
bool platform_boot_request_get(void);
void platform_boot_request_clear(void);
void platform_boot_reset(void);

void platform_boot_error_set(uint32_t error_code);
uint32_t platform_boot_error_get(void);
void platform_boot_error_clear(void);

#endif /* PLATFORM_BOOT_H */