/*
 *  platform_uart.h
 *
 *  Created on: 22 Jul 2026
 *  Author: jochman03
 */

#ifndef PLATFORM_UART_H
#define PLATFORM_UART_H

#include <stdbool.h>
#include <stddef.h>
#include "usart.h"

#define PLATFORM_UART_BUFFER_SIZE 64

extern uint8_t platform_uart_rx_buffer[PLATFORM_UART_BUFFER_SIZE];

typedef void (*platform_uart_receive_callback_t)(const uint8_t* data, size_t length);
typedef void (*platform_uart_transmit_complete_callback_t)(void);

bool platform_uart_transmit(const uint8_t* data, size_t length);
void platform_uart_init();
void platform_uart_set_receive_callback(platform_uart_receive_callback_t receive_callback);
void platform_uart_set_transmit_complete_callback(
    platform_uart_transmit_complete_callback_t transmit_callback);

#endif /* PLATFORM_UART_H */