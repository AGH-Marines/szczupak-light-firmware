#include <stdint.h>
#include "app_faults.h"
#include "platform_boot.h"

void app_faults_fatal(app_fault_type_t type) {
    platform_boot_error_set((uint32_t)type);
    platform_boot_reset();
}

void app_faults_assert(void) {
    app_faults_fatal(APP_FAULT_ASSERT);
}