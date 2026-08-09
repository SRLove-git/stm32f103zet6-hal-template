/**
 ******************************************************************************
 * @file    can_bus.c
 * @brief   CAN1 driver (polling TX/RX).
 ******************************************************************************
 */

#include "can_bus.h"
#include <string.h>

CAN_HandleTypeDef hcan1;

void CAN1_Init(void)
{
    CAN_FilterTypeDef sFilterConfig = {0};

    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 4; /* 36 MHz / (4*18) = 500 kbit/s */
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Pass everything (mask = 0): simple for a template, tighten per project */
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000U;
    sFilterConfig.FilterIdLow = 0x0000U;
    sFilterConfig.FilterMaskIdHigh = 0x0000U;
    sFilterConfig.FilterMaskIdLow = 0x0000U;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
}

HAL_StatusTypeDef CAN1_SendMsg(uint32_t std_id, uint8_t* data, uint8_t len)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t mailbox = 0U;

    if (len > 8U)
    {
        return HAL_ERROR;
    }

    tx_header.StdId = std_id;
    tx_header.ExtId = 0U;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = len;

    return HAL_CAN_AddTxMessage(&hcan1, &tx_header, data, &mailbox);
}

uint8_t CAN1_ReceiveMsg(uint32_t* std_id, uint8_t* data)
{
    CAN_RxHeaderTypeDef rx_header = {0};

    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0U)
    {
        return 0U;
    }

    (void)HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, data);
    *std_id = rx_header.StdId;
    return rx_header.DLC;
}

HAL_StatusTypeDef CAN1_SelfTest(void)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    CAN_RxHeaderTypeDef rx_header;
    uint8_t tx_data[8] = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U, 0x88U};
    uint8_t rx_data[8] = {0U};
    uint32_t mailbox = 0U;
    uint32_t start;
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enter loopback mode */
    if (HAL_CAN_Stop(&hcan1) != HAL_OK)
    {
        return HAL_ERROR;
    }
    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    if ((HAL_CAN_Init(&hcan1) != HAL_OK) || (HAL_CAN_Start(&hcan1) != HAL_OK))
    {
        ret = HAL_ERROR;
        goto restore;
    }

    tx_header.StdId = 0x123U;
    tx_header.ExtId = 0U;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8U;
    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &mailbox) != HAL_OK)
    {
        ret = HAL_ERROR;
        goto restore;
    }

    /* Wait for the loopback echo (poll, max 100 ms) */
    start = HAL_GetTick();
    while ((HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0U) &&
           ((HAL_GetTick() - start) < 100U))
    {
    }

    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0U)
    {
        ret = HAL_ERROR;
        goto restore;
    }

    (void)HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data);
    if ((rx_header.DLC != 8U) || (memcmp(tx_data, rx_data, 8U) != 0))
    {
        ret = HAL_ERROR;
    }

restore:
    /* Back to normal mode */
    (void)HAL_CAN_Stop(&hcan1);
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    if ((HAL_CAN_Init(&hcan1) != HAL_OK) || (HAL_CAN_Start(&hcan1) != HAL_OK))
    {
        ret = HAL_ERROR;
    }

    return ret;
}
