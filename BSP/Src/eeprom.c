/**
 ******************************************************************************
 * @file    eeprom.c
 * @brief   24C02 EEPROM driver over software I2C (PB6/PB7).
 *
 *          Bit-banged I2C is used instead of the STM32F1 hardware I2C
 *          peripheral, which is notoriously unreliable (see the board manual).
 ******************************************************************************
 */

#include "eeprom.h"
#include "bsp_dwt.h"

#define EEPROM_SCL_PORT GPIOB
#define EEPROM_SCL_PIN GPIO_PIN_6
#define EEPROM_SDA_PORT GPIOB
#define EEPROM_SDA_PIN GPIO_PIN_7

static void I2C_Delay(void)
{
    BSP_DWT_DelayUs(5U); /* ~100 kHz */
}

static void SCL_Write(uint8_t level)
{
    HAL_GPIO_WritePin(EEPROM_SCL_PORT, EEPROM_SCL_PIN,
                      (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void SDA_Write(uint8_t level)
{
    HAL_GPIO_WritePin(EEPROM_SDA_PORT, EEPROM_SDA_PIN,
                      (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t SDA_Read(void)
{
    return (HAL_GPIO_ReadPin(EEPROM_SDA_PORT, EEPROM_SDA_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

static void I2C_Start(void)
{
    SDA_Write(1U);
    SCL_Write(1U);
    I2C_Delay();
    SDA_Write(0U);
    I2C_Delay();
    SCL_Write(0U);
    I2C_Delay();
}

static void I2C_Stop(void)
{
    SDA_Write(0U);
    SCL_Write(1U);
    I2C_Delay();
    SDA_Write(1U);
    I2C_Delay();
}

static void I2C_SendByte(uint8_t byte)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        SCL_Write(0U);
        I2C_Delay();
        SDA_Write((byte & 0x80U) != 0U);
        I2C_Delay();
        SCL_Write(1U);
        I2C_Delay();
        byte <<= 1U;
    }
    SCL_Write(0U);
    I2C_Delay();
}

static uint8_t I2C_RecvByte(void)
{
    uint8_t i;
    uint8_t byte = 0U;

    SDA_Write(1U);
    for (i = 0U; i < 8U; i++)
    {
        byte <<= 1U;
        SCL_Write(0U);
        I2C_Delay();
        SCL_Write(1U);
        I2C_Delay();
        if (SDA_Read() != 0U)
        {
            byte |= 0x01U;
        }
    }
    SCL_Write(0U);
    I2C_Delay();
    return byte;
}

static uint8_t I2C_WaitAck(void)
{
    uint8_t ack;

    SDA_Write(1U);
    I2C_Delay();
    SCL_Write(1U);
    I2C_Delay();
    ack = (SDA_Read() == 0U) ? 1U : 0U;
    SCL_Write(0U);
    I2C_Delay();
    return ack;
}

static void I2C_Ack(uint8_t ack)
{
    SDA_Write(ack); /* 1 = NACK, 0 = ACK */
    I2C_Delay();
    SCL_Write(1U);
    I2C_Delay();
    SCL_Write(0U);
    I2C_Delay();
    SDA_Write(1U);
}

/**
 * @brief Software I2C block write (no page splitting).
 */
static HAL_StatusTypeDef SW_Write(uint16_t addr, const uint8_t* buf, uint16_t len)
{
    uint16_t i;
    uint8_t ok;

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    ok = I2C_WaitAck();
    if (ok != 0U)
    {
        I2C_SendByte((uint8_t)addr);
        ok = I2C_WaitAck();
    }
    for (i = 0U; (i < len) && (ok != 0U); i++)
    {
        I2C_SendByte(buf[i]);
        ok = I2C_WaitAck();
    }
    I2C_Stop();
    return (ok != 0U) ? HAL_OK : HAL_ERROR;
}

/**
 * @brief Software I2C block read (random address).
 */
static HAL_StatusTypeDef SW_Read(uint16_t addr, uint8_t* buf, uint16_t len)
{
    uint16_t i;
    uint8_t ok;

    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);
    ok = I2C_WaitAck();
    if (ok != 0U)
    {
        I2C_SendByte((uint8_t)addr);
        ok = I2C_WaitAck();
    }
    if (ok != 0U)
    {
        I2C_Start(); /* repeated start */
        I2C_SendByte(EEPROM_ADDR | 1U);
        ok = I2C_WaitAck();
    }
    for (i = 0U; (i < len) && (ok != 0U); i++)
    {
        buf[i] = I2C_RecvByte();
        I2C_Ack((i == (uint16_t)(len - 1U)) ? 1U : 0U);
    }
    I2C_Stop();
    return (ok != 0U) ? HAL_OK : HAL_ERROR;
}

void EEPROM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    BSP_DWT_DelayInit();

    /* Open-drain outputs: the board provides 4.7k pull-ups */
    GPIO_InitStruct.Pin = EEPROM_SCL_PIN | EEPROM_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    SCL_Write(1U);
    SDA_Write(1U);
}

HAL_StatusTypeDef EEPROM_ReadBuffer(uint16_t addr, uint8_t* buf, uint16_t len)
{
    if ((addr + len) > EEPROM_SIZE)
    {
        return HAL_ERROR;
    }
    return SW_Read(addr, buf, len);
}

HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t addr, const uint8_t* buf, uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_OK;

    if ((addr + len) > EEPROM_SIZE)
    {
        return HAL_ERROR;
    }

    /* Split the write at page boundaries (24C02 page = 8 bytes). */
    while (len > 0U)
    {
        uint16_t page_left = EEPROM_PAGE_SIZE - (addr % EEPROM_PAGE_SIZE);
        uint16_t chunk = (len < page_left) ? len : page_left;

        ret = SW_Write(addr, buf, chunk);
        if (ret != HAL_OK)
        {
            break;
        }

        HAL_Delay(5U); /* EEPROM internal write cycle (max 5 ms) */

        addr += chunk;
        buf += chunk;
        len -= chunk;
    }

    return ret;
}

uint8_t EEPROM_ReadByte(uint16_t addr)
{
    uint8_t data = 0U;
    (void)EEPROM_ReadBuffer(addr, &data, 1U);
    return data;
}

HAL_StatusTypeDef EEPROM_WriteByte(uint16_t addr, uint8_t data)
{
    return EEPROM_WriteBuffer(addr, &data, 1U);
}
