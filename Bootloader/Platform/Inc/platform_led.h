/*
 *  platform_led.h
 *
 *  Created on: 08 Aug 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_LED_H
#define PLATFORM_LED_H

#include <stdbool.h>

void platform_led_status_set(bool state);
void platform_led_status_toggle();

void platform_led_fault_set(bool state);
void platform_led_fault_toggle();

#endif /* PLATFORM_LED_H */