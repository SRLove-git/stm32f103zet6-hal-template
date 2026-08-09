/**
  ******************************************************************************
  * @file    onewire.c
  * @brief   One-wire + DS18B20 driver (bit-banged, polling).
  ******************************************************************************
  */

#include "onewire.h"
#include "bsp_dwt.h"

#define OW_CMD_SKIP_ROM      0xCCU
#define OW_CMD_CONVERT_T     0x44U
#define OW_CMD_READ_SCRATCH  0xBEU

static void OW_WriteByte(uint8_t byte);
static uint8_t OW_ReadByte(void);

void OW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOG_CLK_ENABLE();
    BSP_DWT_DelayInit();

    GPIO_InitStruct.Pin  = OW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; /* open-drain: external 4.7k pull-up */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_SET);
}

static void OW_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = OW_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_PORT, &GPIO_InitStruct);
}

static void OW_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin  = OW_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(OW_PORT, &GPIO_InitStruct);
}

uint8_t OW_Reset(void)
{
    uint8_t presence = 0U;

    OW_SetOutput();
    HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_RESET);
    BSP_DWT_DelayUs(480U);
    HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_SET);
    OW_SetInput();
    BSP_DWT_DelayUs(70U);

    /* Device pulls the bus low to signal presence */
    if (HAL_GPIO_ReadPin(OW_PORT, OW_PIN) == GPIO_PIN_RESET)
    {
        presence = 1U;
    }
    BSP_DWT_DelayUs(410U);
    OW_SetOutput();

    return presence;
}

static void OW_WriteByte(uint8_t byte)
{
    uint8_t i;

    OW_SetOutput();
    for (i = 0U; i < 8U; i++)
    {
        if ((byte & 0x01U) != 0U)
        {
            /* Write '1': low 6 us then release */
            HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_RESET);
            BSP_DWT_DelayUs(6U);
            HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_SET);
            BSP_DWT_DelayUs(64U);
        }
        else
        {
            /* Write '0': low 60 us */
            HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_RESET);
            BSP_DWT_DelayUs(60U);
            HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_SET);
            BSP_DWT_DelayUs(10U);
        }
        byte >>= 1U;
    }
}

static uint8_t OW_ReadByte(void)
{
    uint8_t i;
    uint8_t data = 0U;

    for (i = 0U; i < 8U; i++)
    {
        data >>= 1U;
        OW_SetOutput();
        HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_RESET);
        BSP_DWT_DelayUs(1U);
        HAL_GPIO_WritePin(OW_PORT, OW_PIN, GPIO_PIN_SET);
        OW_SetInput();
        BSP_DWT_DelayUs(14U);

        if (HAL_GPIO_ReadPin(OW_PORT, OW_PIN) == GPIO_PIN_SET)
        {
            data |= 0x80U;
        }
        BSP_DWT_DelayUs(45U);
    }
    OW_SetOutput();
    return data;
}

int16_t DS18B20_GetTemp(void)
{
    uint8_t scratch[9];
    uint8_t i;
    int16_t raw;

    if (OW_Reset() == 0U)
    {
        return 0x7FFFU; /* no device */
    }
    OW_WriteByte(OW_CMD_SKIP_ROM);
    OW_WriteByte(OW_CMD_CONVERT_T);

    /* Wait for conversion (max 750 ms at 12-bit resolution) */
    for (i = 0U; i < 75U; i++)
    {
        HAL_Delay(10U);
    }

    if (OW_Reset() == 0U)
    {
        return 0x7FFFU;
    }
    OW_WriteByte(OW_CMD_SKIP_ROM);
    OW_WriteByte(OW_CMD_READ_SCRATCH);

    for (i = 0U; i < 9U; i++)
    {
        scratch[i] = OW_ReadByte();
    }

    /* CRC check is omitted for brevity; scratch[8] holds it. */
    raw = (int16_t)(((uint16_t)scratch[1] << 8U) | scratch[0]);
    return (int16_t)(((int32_t)raw * 625) / 10); /* 0.0625 degC -> 0.01 degC */
}
