#include <string.h>
#include "fdcan.h"
#include "platform_can.h"

#define PLATFORM_CAN_APP_FILTERID1 0x100U
#define PLATFORM_CAN_APP_FILTERID2 0x7FFU

#define PLATFORM_CAN_BOOT_FILTERID1 0x600U
#define PLATFORM_CAN_BOOT_FILTERID2 0x7FFU

static FDCAN_HandleTypeDef* platform_can = &hfdcan1;
static platform_can_receive_callback_t platform_can_receive_callback;
static platform_can_reset_callback_t platform_can_reset_callback;

static const uint32_t dlc_map[9] = {
    FDCAN_DLC_BYTES_0, FDCAN_DLC_BYTES_1, FDCAN_DLC_BYTES_2, FDCAN_DLC_BYTES_3, FDCAN_DLC_BYTES_4,
    FDCAN_DLC_BYTES_5, FDCAN_DLC_BYTES_6, FDCAN_DLC_BYTES_7, FDCAN_DLC_BYTES_8,
};

void platform_can_init(void) {
    FDCAN_FilterTypeDef app_filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_MASK,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = PLATFORM_CAN_APP_FILTERID1,
        .FilterID2 = PLATFORM_CAN_APP_FILTERID2,
    };
    FDCAN_FilterTypeDef boot_filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 1,
        .FilterType = FDCAN_FILTER_MASK,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = PLATFORM_CAN_BOOT_FILTERID1,
        .FilterID2 = PLATFORM_CAN_BOOT_FILTERID2,
    };

    HAL_FDCAN_ConfigFilter(platform_can, &app_filter);
    HAL_FDCAN_ConfigFilter(platform_can, &boot_filter);

    HAL_FDCAN_ConfigGlobalFilter(platform_can, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE,
                                 FDCAN_REJECT_REMOTE);
    HAL_FDCAN_ActivateNotification(platform_can, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_BUS_OFF,
                                   0);

    HAL_FDCAN_Start(platform_can);
}

bool platform_can_transmit(const can_frame_t* frame) {
    if ((frame == NULL) || (frame->dlc > 8U)) {
        return false;
    }

    FDCAN_TxHeaderTypeDef header = {
        .Identifier = frame->id,
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = dlc_map[frame->dlc],
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0,
    };

    if (HAL_FDCAN_AddMessageToTxFifoQ(platform_can, &header, frame->data) == HAL_OK) {
        return true;
    }
    return false;
}

void platform_can_set_receive_callback(platform_can_receive_callback_t cb) {
    platform_can_receive_callback = cb;
}

void platform_can_set_reset_callback(platform_can_reset_callback_t cb) {
    platform_can_reset_callback = cb;
}

void platform_can_reset(void) {
    HAL_FDCAN_Stop(platform_can);
    HAL_FDCAN_Start(platform_can);
    HAL_FDCAN_ActivateNotification(platform_can, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_BUS_OFF,
                                   0);
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
    if (hfdcan != platform_can) {
        return;
    }
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U) {
        return;
    }

    FDCAN_RxHeaderTypeDef receive_header;
    uint8_t data[8];

    while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &receive_header, data) == HAL_OK) {
        uint8_t dlc = (uint8_t)receive_header.DataLength;
        if (dlc > 8U) {
            continue;
        }
        can_frame_t frame = {
            .id = receive_header.Identifier,
            .dlc = dlc,
        };
        memcpy(frame.data, data, dlc);
        if (platform_can_receive_callback != NULL) {
            platform_can_receive_callback(&frame);
        }
    }
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t ErrorStatusITs) {
    if (hfdcan != platform_can) {
        return;
    }
    if ((ErrorStatusITs & FDCAN_FLAG_BUS_OFF) == 0U) {
        return;
    }
    FDCAN_ProtocolStatusTypeDef status;
    HAL_FDCAN_GetProtocolStatus(platform_can, &status);

    if (status.BusOff != 0U && platform_can_reset_callback != NULL) {
        platform_can_reset_callback();
    }
}