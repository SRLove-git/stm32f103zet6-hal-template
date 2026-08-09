/**
  ******************************************************************************
  * @file    sd_card.c
  * @brief   TF card driver via HAL SDIO (4-bit bus).
  ******************************************************************************
  */

#include "sd_card.h"

#define SD_TIMEOUT 30000U /* ms */

SD_HandleTypeDef hsd;

void SD_Init(void)
{
    HAL_SD_CardInfoTypeDef card_info;

    hsd.Instance                 = SDIO;
    hsd.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.ClockDiv            = 2; /* ~18 MHz from 72 MHz PCLK2, within 24 MHz max */
    hsd.Init.BusWide             = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;

    if (HAL_SD_Init(&hsd) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
    {
        Error_Handler();
    }

    (void)HAL_SD_GetCardInfo(&hsd, &card_info);
}

HAL_StatusTypeDef SD_ReadBlocks(uint32_t block_addr, uint8_t *buf, uint32_t count)
{
    uint32_t state = SD_GetCardState();
    if (state != HAL_SD_CARD_TRANSFER)
    {
        return HAL_ERROR;
    }
    return HAL_SD_ReadBlocks(&hsd, buf, block_addr, count, SD_TIMEOUT);
}

HAL_StatusTypeDef SD_WriteBlocks(uint32_t block_addr, const uint8_t *buf, uint32_t count)
{
    uint32_t state = SD_GetCardState();
    if (state != HAL_SD_CARD_TRANSFER)
    {
        return HAL_ERROR;
    }
    return HAL_SD_WriteBlocks(&hsd, (uint8_t *)buf, block_addr, count, SD_TIMEOUT);
}

HAL_StatusTypeDef SD_GetCardInfo(HAL_SD_CardInfoTypeDef *info)
{
    return HAL_SD_GetCardInfo(&hsd, info);
}

uint32_t SD_GetCardState(void)
{
    return HAL_SD_GetCardState(&hsd);
}
