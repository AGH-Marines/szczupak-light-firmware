#include <string.h>
#include "fdcan.h"
#include "can.h"
#include "platform_can.h"

#define FILTERID1     0x600U
#define CAN_FILTERID2 0x601U

#define CAN_TX_FIFO_SIZE 3U

static FDCAN_HandleTypeDef* platform_can = &hfdcan1;

static const uint32_t dlc_map[9] = {
    FDCAN_DLC_BYTES_0, FDCAN_DLC_BYTES_1, FDCAN_DLC_BYTES_2, FDCAN_DLC_BYTES_3, FDCAN_DLC_BYTES_4,
    FDCAN_DLC_BYTES_5, FDCAN_DLC_BYTES_6, FDCAN_DLC_BYTES_7, FDCAN_DLC_BYTES_8,
};

void platform_can_init(void) {
    FDCAN_FilterTypeDef filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_DUAL,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = FILTERID1,
        .FilterID2 = CAN_FILTERID2,
    };
    HAL_FDCAN_ConfigFilter(platform_can, &filter);
    HAL_FDCAN_ConfigGlobalFilter(platform_can, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE,
                                 FDCAN_REJECT_REMOTE);
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

bool platform_can_receive(can_frame_t* frame) {
    if (frame == NULL) {
        return false;
    }
    if (HAL_FDCAN_GetRxFifoFillLevel(platform_can, FDCAN_RX_FIFO0) == 0U) {
        return false;
    }

    FDCAN_RxHeaderTypeDef receive_header;
    uint8_t data[8];

    if (HAL_FDCAN_GetRxMessage(platform_can, FDCAN_RX_FIFO0, &receive_header, data) != HAL_OK) {
        return false;
    }
    uint8_t dlc = (uint8_t)receive_header.DataLength;
    if (dlc > 8U) {
        return false;
    }
    frame->id = receive_header.Identifier;
    frame->dlc = dlc;
    memcpy(frame->data, data, dlc);
    return true;
}

bool platform_can_tx_fifo_empty(void) {
    return HAL_FDCAN_GetTxFifoFreeLevel(platform_can) == CAN_TX_FIFO_SIZE;
}
