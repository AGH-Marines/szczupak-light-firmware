#include <stdint.h>
#include "faults.h"

#include "platform_led.h"

#define SYSTEM_CORE_CLOCK 32000000U

static void fault_delay_ms(uint32_t ms) {
    uint32_t loops = (SYSTEM_CORE_CLOCK / 10000U) * ms;
    for (volatile uint32_t i = 0U; i < loops; ++i) {
    }
}

void faults_handle(fault_type type) {
    while (1) {
        if ((uint8_t)type > 0U) {
            for (int i = 0U; i < (2 * (uint8_t)type); ++i) {
                platform_led_fault_toggle();
                fault_delay_ms(200U);
            }
        } else {
            platform_led_fault_set(true);
        }
        fault_delay_ms(1000U);
    }
}

void faults_assert_handle(void) {
    faults_handle(FAULT_ASSERT);
}