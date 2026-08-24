/*
 *  platform_usb.h
 *
 *  Created on: 24 Aug 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_USB_H
#define PLATFORM_USB_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*platform_usb_receive_callback_t)(const uint8_t* data, size_t length);
typedef void (*platform_usb_transmit_complete_callback_t)(void);

void platform_usb_init(void);
bool platform_usb_transmit(const uint8_t* data, size_t length);
void platform_usb_set_receive_callback(platform_usb_receive_callback_t callback);
void platform_usb_set_transmit_complete_callback(
    platform_usb_transmit_complete_callback_t callback);

void platform_usb_receive_isr(const uint8_t* data, size_t length);
void platform_usb_transmit_complete_isr(void);

#endif /* PLATFORM_USB_H */