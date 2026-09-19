#include "platform_led.h"
#include "main.h"
#include "gpio.h"

void platform_led_status_set(bool state) {
    HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, (state ? RESET : SET));
}

void platform_led_status_toggle() {
    HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
}

void platform_led_fault_set(bool state) {
    HAL_GPIO_WritePin(LED_FAULT_GPIO_Port, LED_FAULT_Pin, (state ? RESET : SET));
}

void platform_led_fault_toggle() {
    HAL_GPIO_TogglePin(LED_FAULT_GPIO_Port, LED_FAULT_Pin);
}
