/*
 *  platform_memory.h
 *
 *  Created on: 18 Sep 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_MEMORY_H
#define PLATFORM_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

bool platform_memory_app_valid(void);
void platform_memory_app_start(void);
uint32_t platform_memory_app_size(void);

bool platform_memory_metadata_clear(void);
bool platform_memory_metadata_save(uint32_t image_size, uint32_t image_crc);

bool platform_memory_app_clear(uint32_t image_size);
bool platform_memory_app_write(uint32_t offset, const uint8_t* data, uint32_t size);
bool platform_memory_app_calculate_crc(uint32_t image_size, uint32_t* crc);
#endif /* PLATFORM_MEMORY_H */