/**
 ******************************************************************************
 * @file    sw_i2c.c
 * @brief   Bit-banged I2C master implementation (~100 kHz).
 ******************************************************************************
 */

#include "sw_i2c.h"
#include "bsp_dwt.h"

static void SW_I2C_Delay(void)
{
    BSP_DWT_DelayUs(5U);
}

static void SCL_Write(SW_I2C_t* i2c, uint8_t level)
{
    HAL_GPIO_WritePin(i2c->scl_port, i2c->scl_pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void SDA_Write(SW_I2C_t* i2c, uint8_t level)
{
    HAL_GPIO_WritePin(i2c->sda_port, i2c->sda_pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t SDA_Read(SW_I2C_t* i2c)
{
    return (HAL_GPIO_ReadPin(i2c->sda_port, i2c->sda_pin) == GPIO_PIN_SET) ? 1U : 0U;
}

void SW_I2C_Init(SW_I2C_t* i2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    BSP_DWT_DelayInit();

    GPIO_InitStruct.Pin = i2c->scl_pin | i2c->sda_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(i2c->scl_port, &GPIO_InitStruct);

    SCL_Write(i2c, 1U);
    SDA_Write(i2c, 1U);
}

void SW_I2C_Start(SW_I2C_t* i2c)
{
    SDA_Write(i2c, 1U);
    SCL_Write(i2c, 1U);
    SW_I2C_Delay();
    SDA_Write(i2c, 0U);
    SW_I2C_Delay();
    SCL_Write(i2c, 0U);
    SW_I2C_Delay();
}

void SW_I2C_Stop(SW_I2C_t* i2c)
{
    SDA_Write(i2c, 0U);
    SCL_Write(i2c, 1U);
    SW_I2C_Delay();
    SDA_Write(i2c, 1U);
    SW_I2C_Delay();
}

uint8_t SW_I2C_WriteByte(SW_I2C_t* i2c, uint8_t byte)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0U; i < 8U; i++)
    {
        SCL_Write(i2c, 0U);
        SW_I2C_Delay();
        SDA_Write(i2c, (byte & 0x80U) != 0U);
        SW_I2C_Delay();
        SCL_Write(i2c, 1U);
        SW_I2C_Delay();
        byte <<= 1U;
    }

    SCL_Write(i2c, 0U);
    SW_I2C_Delay();
    SDA_Write(i2c, 1U);
    SW_I2C_Delay();
    SCL_Write(i2c, 1U);
    SW_I2C_Delay();
    ack = (SDA_Read(i2c) == 0U) ? 1U : 0U;
    SCL_Write(i2c, 0U);
    SW_I2C_Delay();

    return ack;
}

uint8_t SW_I2C_ReadByte(SW_I2C_t* i2c)
{
    uint8_t i;
    uint8_t byte = 0U;

    SDA_Write(i2c, 1U);
    for (i = 0U; i < 8U; i++)
    {
        byte <<= 1U;
        SCL_Write(i2c, 0U);
        SW_I2C_Delay();
        SCL_Write(i2c, 1U);
        SW_I2C_Delay();
        if (SDA_Read(i2c) != 0U)
        {
            byte |= 0x01U;
        }
    }
    SCL_Write(i2c, 0U);
    SW_I2C_Delay();
    return byte;
}

void SW_I2C_Ack(SW_I2C_t* i2c, uint8_t nack)
{
    SDA_Write(i2c, nack);
    SW_I2C_Delay();
    SCL_Write(i2c, 1U);
    SW_I2C_Delay();
    SCL_Write(i2c, 0U);
    SW_I2C_Delay();
    SDA_Write(i2c, 1U);
}
