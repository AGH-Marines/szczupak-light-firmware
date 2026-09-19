#include <stdint.h>
#include <stdbool.h>

#include "stm32g0xx.h"
#include "platform_memory.h"

#define PLATFORM_MEMORY_APP_START 0x08004000U
#define PLATFORM_MEMORY_APP_END   0x0801F800U

#define PLATFORM_MEMORY_RAM_START 0x20000000U
#define PLATFORM_MEMORY_RAM_END   0x20024000U

#define PLATFORM_MEMORY_METADATA_START 0x0801F800U

typedef void (*app_entry_t)(void);

static const uint32_t* const vector_table = (const uint32_t*)PLATFORM_MEMORY_APP_START;

bool platform_memory_app_valid(void) {
    uint32_t initial_msp = vector_table[0];
    uint32_t reset_vector = vector_table[1];
    uint32_t reset_handler_address = reset_vector & ~1U;

    if ((reset_vector & 1U) == 0U) {
        return false;
    }
    if (!(initial_msp >= PLATFORM_MEMORY_RAM_START && initial_msp <= PLATFORM_MEMORY_RAM_END)) {
        return false;
    }
    if (!(reset_handler_address >= PLATFORM_MEMORY_APP_START &&
          reset_handler_address < PLATFORM_MEMORY_APP_END)) {
        return false;
    }
    return true;
}

void platform_memory_app_start(void) {
    uint32_t initial_msp = vector_table[0];
    uint32_t reset_vector = vector_table[1];
    app_entry_t app_entry = (app_entry_t)reset_vector;

    __disable_irq();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;

    NVIC->ICER[0] = 0xFFFFFFFFU;
    NVIC->ICPR[0] = 0xFFFFFFFFU;

    SCB->VTOR = PLATFORM_MEMORY_APP_START;

    __DSB();
    __ISB();

    __enable_irq();

    __set_MSP(initial_msp);
    app_entry();
}