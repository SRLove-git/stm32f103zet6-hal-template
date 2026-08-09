/**
 ******************************************************************************
 * @file    w25qxx.c
 * @brief   W25Q128 SPI Flash driver.
 ******************************************************************************
 */

#include "w25qxx.h"

SPI_HandleTypeDef hspi2;

/* Command set (W25Q series) */
#define W25X_CMD_WRITE_ENABLE 0x06U
#define W25X_CMD_READ_STATUS 0x05U
#define W25X_CMD_READ_DATA 0x03U
#define W25X_CMD_PAGE_PROGRAM 0x02U
#define W25X_CMD_SECTOR_ERASE 0x20U
#define W25X_CMD_CHIP_ERASE 0xC7U
#define W25X_CMD_READ_JEDEC_ID 0x9FU

#define W25X_STATUS_BUSY 0x01U

static void W25QXX_CS_Low(void)
{
    HAL_GPIO_WritePin(W25QXX_CS_PORT, W25QXX_CS_PIN, GPIO_PIN_RESET);
}

static void W25QXX_CS_High(void)
{
    HAL_GPIO_WritePin(W25QXX_CS_PORT, W25QXX_CS_PIN, GPIO_PIN_SET);
}

static uint8_t W25QXX_SpiByte(uint8_t byte)
{
    uint8_t rx = 0U;
    if (HAL_SPI_TransmitReceive(&hspi2, &byte, &rx, 1U, W25QXX_TIMEOUT) != HAL_OK)
    {
        Error_Handler();
    }
    return rx;
}

static void W25QXX_WriteEnable(void)
{
    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_WRITE_ENABLE);
    W25QXX_CS_High();
}

static void W25QXX_WaitBusy(void)
{
    uint8_t status;

    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_READ_STATUS);
    do
    {
        status = W25QXX_SpiByte(0xFFU);
    } while ((status & W25X_STATUS_BUSY) != 0U);
    W25QXX_CS_High();
}

void W25QXX_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* CS: plain GPIO output, idle high */
    GPIO_InitStruct.Pin = W25QXX_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(W25QXX_CS_PORT, &GPIO_InitStruct);
    W25QXX_CS_High();

    hspi2.Instance = W25QXX_SPI;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; /* 18 MHz @ APB1 36 MHz */
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10U;

    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();
    }
}

uint32_t W25QXX_ReadID(void)
{
    uint32_t id = 0U;

    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_READ_JEDEC_ID);
    id = (uint32_t)W25QXX_SpiByte(0xFFU) << 16U;
    id |= (uint32_t)W25QXX_SpiByte(0xFFU) << 8U;
    id |= (uint32_t)W25QXX_SpiByte(0xFFU);
    W25QXX_CS_High();

    return id;
}

void W25QXX_ReadData(uint32_t addr, uint8_t* buf, uint32_t len)
{
    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_READ_DATA);
    (void)W25QXX_SpiByte((uint8_t)(addr >> 16U));
    (void)W25QXX_SpiByte((uint8_t)(addr >> 8U));
    (void)W25QXX_SpiByte((uint8_t)addr);

    while (len-- > 0U)
    {
        *buf++ = W25QXX_SpiByte(0xFFU);
    }
    W25QXX_CS_High();
}

HAL_StatusTypeDef W25QXX_WriteData(uint32_t addr, const uint8_t* buf, uint32_t len)
{
    uint32_t i;

    if ((addr + len) > (16U * 1024U * 1024U))
    {
        return HAL_ERROR;
    }

    /* Note: no automatic erase - call W25QXX_EraseSector() first. */
    while (len > 0U)
    {
        uint32_t page_left = W25QXX_PAGE_SIZE - (addr % W25QXX_PAGE_SIZE);
        uint32_t chunk = (len < page_left) ? len : page_left;

        W25QXX_WriteEnable();
        W25QXX_CS_Low();
        (void)W25QXX_SpiByte(W25X_CMD_PAGE_PROGRAM);
        (void)W25QXX_SpiByte((uint8_t)(addr >> 16U));
        (void)W25QXX_SpiByte((uint8_t)(addr >> 8U));
        (void)W25QXX_SpiByte((uint8_t)addr);
        for (i = 0U; i < chunk; i++)
        {
            (void)W25QXX_SpiByte(*buf++);
        }
        W25QXX_CS_High();
        W25QXX_WaitBusy();

        addr += chunk;
        len -= chunk;
    }

    return HAL_OK;
}

HAL_StatusTypeDef W25QXX_EraseSector(uint32_t addr)
{
    W25QXX_WriteEnable();
    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_SECTOR_ERASE);
    (void)W25QXX_SpiByte((uint8_t)(addr >> 16U));
    (void)W25QXX_SpiByte((uint8_t)(addr >> 8U));
    (void)W25QXX_SpiByte((uint8_t)addr);
    W25QXX_CS_High();
    W25QXX_WaitBusy();
    return HAL_OK;
}

HAL_StatusTypeDef W25QXX_EraseChip(void)
{
    W25QXX_WriteEnable();
    W25QXX_CS_Low();
    (void)W25QXX_SpiByte(W25X_CMD_CHIP_ERASE);
    W25QXX_CS_High();
    W25QXX_WaitBusy();
    return HAL_OK;
}
