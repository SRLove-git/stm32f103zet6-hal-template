/**
 ******************************************************************************
 * @file    w25qxx.h
 * @brief   On-board SPI NOR Flash driver (W25Q128 / 16 MB).
 *
 *          CS   - PB12 | SPI2 SCK  - PB13
 *          MISO - PB14 | SPI2 MOSI - PB15
 *
 * @note    SPI2 is shared with the NRF24L01 wireless interface - keep only
 *          one chip-select active at a time.
 ******************************************************************************
 */

#ifndef __W25QXX_H
#define __W25QXX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define W25QXX_SPI SPI2
#define W25QXX_CS_PORT GPIOB
#define W25QXX_CS_PIN GPIO_PIN_12

#define W25QXX_PAGE_SIZE 256U
#define W25QXX_SECTOR_SIZE 4096U
#define W25QXX_TIMEOUT 100U

    extern SPI_HandleTypeDef hspi2;

    void W25QXX_Init(void);
    uint32_t W25QXX_ReadID(void); /* JEDEC: MF + type + capacity */
    void W25QXX_ReadData(uint32_t addr, uint8_t* buf, uint32_t len);
    HAL_StatusTypeDef W25QXX_WriteData(uint32_t addr, const uint8_t* buf, uint32_t len);
    HAL_StatusTypeDef W25QXX_EraseSector(uint32_t addr);
    HAL_StatusTypeDef W25QXX_EraseChip(void);

#ifdef __cplusplus
}
#endif

#endif /* __W25QXX_H */
