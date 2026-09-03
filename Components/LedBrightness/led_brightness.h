/*
 *  led_brightness.h
 *
 *  Created on: 01 Sep 2026
 *  Author: jochman03
 */

#ifndef LED_BRIGHTNESS_H
#define LED_BRIGHTNESS_H

#include <stdint.h>

uint16_t led_brightness_to_pulse(uint8_t percent);
uint8_t led_brightness_clamp_percent(uint8_t percent);

#endif /* LED_BRIGHTNESS_H */
