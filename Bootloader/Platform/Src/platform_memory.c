#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32g0xx.h"
#include "crc.h"
#include "platform_memory.h"

#define METADATA_MAGIC 0x360555127546AAAAULL

typedef void (*app_entry_t)(void);

typedef struct platform_memory_metadata_t {
    uint32_t image_size;
    uint32_t image_crc;
    uint64_t magic;
} platform_memory_metadata_t;

_Static_assert(sizeof(platform_memory_metadata_t) == 16U, "UNexpected metadata size");

extern uint8_t __ram_start__[];
extern uint8_t __ram_end__[];
extern uint8_t __app_start__[];
extern uint8_t __app_end__[];
extern uint8_t __metadata_start__[];
extern uint8_t __metadata_end__[];

static const uint32_t* const vector_table = (const uint32_t*)__app_start__;

static bool metadata_valid(void);

bool platform_memory_app_valid(void) {
    if (!metadata_valid()) {
        return false;
    }

    uint32_t initial_msp = vector_table[0];
    uint32_t reset_vector = vector_table[1];
    uint32_t reset_handler_address = reset_vector & ~1U;

    uintptr_t ram_start = (uintptr_t)__ram_start__;
    uintptr_t ram_end = (uintptr_t)__ram_end__;
    uintptr_t app_start = (uintptr_t)__app_start__;

    if ((reset_vector & 1U) == 0U) {
        return false;
    }
    if (!(initial_msp >= ram_start && initial_msp <= ram_end)) {
        return false;
    }
    const platform_memory_metadata_t* metadata =
        (const platform_memory_metadata_t*)__metadata_start__;

    uintptr_t image_end = app_start + metadata->image_size;
    if (!(reset_handler_address >= app_start && reset_handler_address < image_end)) {
        return false;
    }

    uint32_t app_crc = 0U;

    if (!platform_memory_app_calculate_crc(metadata->image_size, &app_crc)) {
        return false;
    }
    if (app_crc != metadata->image_crc) {
        return false;
    }

    return true;
}

void platform_memory_app_start(void) {
    uint32_t initial_msp = vector_table[0];
    uint32_t reset_vector = vector_table[1];
    app_entry_t app_entry = (app_entry_t)reset_vector;
    uintptr_t app_start = (uintptr_t)__app_start__;

    __disable_irq();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;

    NVIC->ICER[0] = 0xFFFFFFFFU;
    NVIC->ICPR[0] = 0xFFFFFFFFU;

    SCB->VTOR = app_start;

    __DSB();
    __ISB();

    __enable_irq();

    __set_MSP(initial_msp);
    app_entry();
}

uint32_t platform_memory_app_size(void) {
    uintptr_t app_start = (uintptr_t)__app_start__;
    uintptr_t app_end = (uintptr_t)__app_end__;

    return (uint32_t)(app_end - app_start);
}

bool platform_memory_metadata_clear(void) {
    uintptr_t metadata_start = (uintptr_t)__metadata_start__;
    uintptr_t metadata_end = (uintptr_t)__metadata_end__;

    uint32_t metadata_size = metadata_end - metadata_start;
    uint32_t first_page = (metadata_start - FLASH_BASE) / FLASH_PAGE_SIZE;
    uint32_t page_count = metadata_size / FLASH_PAGE_SIZE;

    FLASH_EraseInitTypeDef erase_init = {.Banks = FLASH_BANK_1,
                                         .NbPages = page_count,
                                         .Page = first_page,
                                         .TypeErase = FLASH_TYPEERASE_PAGES};

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }
    uint32_t page_error = 0xFFFFFFFFU;
    bool erase_ok =
        HAL_FLASHEx_Erase(&erase_init, &page_error) == HAL_OK && page_error == 0xFFFFFFFFU;

    if (HAL_FLASH_Lock() != HAL_OK) {
        erase_ok = false;
    }

    return erase_ok;
}

bool platform_memory_metadata_save(uint32_t image_size, uint32_t image_crc) {
    uintptr_t metadata_start = (uintptr_t)__metadata_start__;
    uintptr_t metadata_magic = metadata_start + sizeof(uint64_t);

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }

    uint64_t payload = ((uint64_t)image_crc << 32U) | (uint64_t)image_size;
    bool result = (HAL_FLASH_Program(TYPEPROGRAM_DOUBLEWORD, metadata_start, payload) == HAL_OK);
    if (result) {
        result =
            (HAL_FLASH_Program(TYPEPROGRAM_DOUBLEWORD, metadata_magic, METADATA_MAGIC) == HAL_OK);
    }
    bool lock_result = (HAL_FLASH_Lock() == HAL_OK);

    if (!result || !lock_result) {
        return false;
    }

    const platform_memory_metadata_t* metadata =
        (const platform_memory_metadata_t*)__metadata_start__;

    if (metadata->image_size != image_size || metadata->image_crc != image_crc ||
        metadata->magic != METADATA_MAGIC) {
        return false;
    }

    return true;
}

bool platform_memory_app_clear(uint32_t image_size) {
    if (image_size == 0U || image_size > platform_memory_app_size()) {
        return false;
    }
    uint32_t page_count = (image_size + (FLASH_PAGE_SIZE - 1U)) / FLASH_PAGE_SIZE;

    uintptr_t app_start = (uintptr_t)__app_start__;

    uint32_t first_page = (app_start - FLASH_BASE) / FLASH_PAGE_SIZE;

    FLASH_EraseInitTypeDef erase_init = {.Banks = FLASH_BANK_1,
                                         .NbPages = page_count,
                                         .Page = first_page,
                                         .TypeErase = FLASH_TYPEERASE_PAGES};
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }
    uint32_t page_error = 0xFFFFFFFFU;
    bool erase_ok =
        HAL_FLASHEx_Erase(&erase_init, &page_error) == HAL_OK && page_error == 0xFFFFFFFFU;

    if (HAL_FLASH_Lock() != HAL_OK) {
        erase_ok = false;
    }

    return erase_ok;
}

bool platform_memory_app_write(uint32_t offset, const uint8_t* data, uint32_t size) {
    if (data == NULL || size == 0U) {
        return false;
    }
    if ((offset % sizeof(uint64_t)) != 0U) {
        return false;
    }

    uint32_t app_size = platform_memory_app_size();
    if (offset > app_size || size > app_size || size > (app_size - offset)) {
        return false;
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }

    bool result = true;
    uintptr_t app_start = (uintptr_t)__app_start__;

    uint32_t written = 0U;
    while (written < size) {
        uint64_t payload = UINT64_MAX;

        uint32_t payload_size = (size - written);
        if (payload_size > sizeof(payload)) {
            payload_size = sizeof(payload);
        }

        memcpy(&payload, data + written, payload_size);

        result = (HAL_FLASH_Program(TYPEPROGRAM_DOUBLEWORD, app_start + offset + written,
                                    payload) == HAL_OK);
        if (!result) {
            break;
        }

        written += sizeof(payload);
    }

    if (HAL_FLASH_Lock() != HAL_OK) {
        result = false;
    }

    return result;
}

bool platform_memory_app_calculate_crc(uint32_t image_size, uint32_t* crc) {
    if (crc == NULL || image_size == 0U || image_size > platform_memory_app_size()) {
        return false;
    }

    uint32_t crc_value = HAL_CRC_Calculate(&hcrc, (uint32_t*)__app_start__, image_size);
    *crc = ~crc_value;
    return true;
}

static bool metadata_valid(void) {
    const platform_memory_metadata_t* metadata =
        (const platform_memory_metadata_t*)__metadata_start__;
    if (metadata->magic != METADATA_MAGIC) {
        return false;
    }

    if (metadata->image_size == 0U || metadata->image_size > platform_memory_app_size()) {
        return false;
    }

    return true;
}