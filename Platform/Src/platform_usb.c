#include <stdint.h>

#include "platform_usb.h"

#include "usbd_cdc_if.h"

static platform_usb_receive_callback_t rx_callback;
static platform_usb_transmit_complete_callback_t tx_complete_callback;

extern USBD_HandleTypeDef hUsbDeviceFS;

void platform_usb_init(void) {}

bool platform_usb_transmit(const uint8_t* data, size_t length) {
    if ((data == NULL) || (length == 0U) || (length > UINT16_MAX)) {
        return false;
    }

    return CDC_Transmit_FS((uint8_t*)data, (uint16_t)length) == USBD_OK;
}

void platform_usb_set_receive_callback(platform_usb_receive_callback_t callback) {
    if (callback == NULL) {
        return;
    }
    rx_callback = callback;
}

void platform_usb_set_transmit_complete_callback(
    platform_usb_transmit_complete_callback_t callback) {
    if (callback == NULL) {
        return;
    }
    tx_complete_callback = callback;
}

void platform_usb_receive_isr(const uint8_t* data, size_t length) {
    if ((rx_callback == NULL) || (data == NULL) || (length == 0U)) {
        return;
    }
    rx_callback(data, length);
}

void platform_usb_transmit_complete_isr(void) {
    if (tx_complete_callback != NULL) {
        tx_complete_callback();
    }
}

void platform_usb_abort_transmission(void) {
    USBD_CDC_HandleTypeDef* usb_cdc_handle;
    usb_cdc_handle = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (usb_cdc_handle == NULL) {
        return;
    }
    usb_cdc_handle->TxState = 0;
}
