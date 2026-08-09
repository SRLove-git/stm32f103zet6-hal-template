/**
 ******************************************************************************
 * @file    sd_card.h
 * @brief   On-board TF (microSD) card driver - SDIO 4-bit mode.
 *
 *          D0..D3 - PC8..PC11 | SCK - PC12 | CMD - PD2
 *
 * @note    Block size is 512 bytes. Polling access; DMA can be added later.
 ******************************************************************************
 */

#ifndef __SD_CARD_H
#define __SD_CARD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define SD_BLOCK_SIZE 512U

    extern SD_HandleTypeDef hsd;

    void SD_Init(void);
    HAL_StatusTypeDef SD_ReadBlocks(uint32_t block_addr, uint8_t* buf, uint32_t count);
    HAL_StatusTypeDef SD_WriteBlocks(uint32_t block_addr, const uint8_t* buf, uint32_t count);
    HAL_StatusTypeDef SD_GetCardInfo(HAL_SD_CardInfoTypeDef* info);
    uint32_t SD_GetCardState(void);

#ifdef __cplusplus
}
#endif

#endif /* __SD_CARD_H */
