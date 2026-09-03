#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "app.h"
#include "debugger.h"
#include "voltage_sense.h"
#include "current_sense.h"

#include "platform_uart.h"
#include "platform_adc.h"
#include "platform_led.h"

#include "task_diagnostics.h"
#include "task_debug.h"
#include "task_led.h"
#include "task_usb.h"
#include "task_can.h"

#include "FreeRTOS.h"
#include "task.h"

#if DEBUGGER_ENABLED
static const char tag[] = "APP";
#endif /* DEBUGGER_ENABLED */

static void app_task(void* pvParameters);

void app_init(void) {

    task_diagnostics_init();
    task_usb_init();
    task_can_init();
    task_led_init();

#if DEBUGGER_ENABLED
    task_debug_init();
#endif /* DEBUGGER_ENABLED */

    xTaskCreate(app_task, "APP_TASK", 300, NULL, 1, NULL);
}

static void app_task(void* pvParameters) {
    debugger_log_info(tag, "init completed");

    bool status_led_state = false;
    while (1) {
        platform_led_status_set(status_led_state);
        status_led_state = !status_led_state;
        debugger_log_info(tag, "changing led state");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
