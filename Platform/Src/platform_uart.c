#include <stdint.h>

#include "platform_uart.h"
#include "dma.h"
#include "usart.h"

UART_HandleTypeDef* platform_uart = &huart5;

uint8_t platform_uart_rx_buffer[PLATFORM_UART_BUFFER_SIZE];

static volatile size_t rx_position;
static platform_uart_receive_callback_t rx_callback;
static platform_uart_transmit_complete_callback_t tx_complete_callback;

bool platform_uart_transmit(const uint8_t* data, size_t length) {
    if ((data == NULL) || (length == 0U) || (length > UINT16_MAX)) {
        return false;
    }
    return HAL_UART_Transmit_DMA(platform_uart, (uint8_t*)data, (uint16_t)length) == HAL_OK;
}

void platform_uart_set_receive_callback(platform_uart_receive_callback_t receive_callback) {
    if (receive_callback == NULL) {
        return;
    }
    rx_callback = receive_callback;
}

void platform_uart_set_transmit_complete_callback(
    platform_uart_transmit_complete_callback_t transmit_callback) {
    if (transmit_callback == NULL) {
        return;
    }
    tx_complete_callback = transmit_callback;
}

void platform_uart_abort_transmission(void) {
    HAL_UART_AbortTransmit(platform_uart);
}

void platform_uart_init(void) {
    rx_position = 0;

    HAL_UARTEx_ReceiveToIdle_DMA(platform_uart, platform_uart_rx_buffer,
                                 sizeof(platform_uart_rx_buffer));
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* uart, uint16_t position) {
    if ((uart != platform_uart) || (rx_callback == NULL)) {
        return;
    }

    if (position > rx_position) {
        rx_callback(&platform_uart_rx_buffer[rx_position], (size_t)position - rx_position);
    }

    else if (position < rx_position) {
        rx_callback(&platform_uart_rx_buffer[rx_position], PLATFORM_UART_BUFFER_SIZE - rx_position);

        if (position > 0) {
            rx_callback(platform_uart_rx_buffer, position);
        }
    }

    rx_position = position;

    if (rx_position == PLATFORM_UART_BUFFER_SIZE) {
        rx_position = 0;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart != platform_uart) {
        return;
    }
    if (tx_complete_callback != NULL) {
        tx_complete_callback();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
    if (huart != platform_uart) {
        return;
    }

    __HAL_UART_CLEAR_OREFLAG(huart);
    rx_position = 0;

    platform_uart_abort_transmission();

    if (tx_complete_callback != NULL) {
        tx_complete_callback();
    }

    HAL_UARTEx_ReceiveToIdle_DMA(platform_uart, platform_uart_rx_buffer,
                                 sizeof(platform_uart_rx_buffer));
}