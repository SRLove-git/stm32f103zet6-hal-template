/**
 ******************************************************************************
 * @file    eeprom.c
 * @brief   24C02 EEPROM driver over software I2C (PB6/PB7).
 *
 *          Bit-banged I2C (shared sw_i2c module) is used instead of the
 *          STM32F1 hardware I2C peripheral, which is notoriously unreliable.
 ******************************************************************************
 */

#include "eeprom.h"
#include "sw_i2c.h"

static SW_I2C_t eeprom_i2c = {GPIOB, GPIO_PIN_6, GPIOB, GPIO_PIN_7};

/**
 * @brief Software I2C block write (no page splitting).
 */
static HAL_StatusTypeDef SW_Write(uint16_t addr, const uint8_t* buf, uint16_t len)
{
    uint16_t i;
    uint8_t ok;

    SW_I2C_Start(&eeprom_i2c);
    ok = SW_I2C_WriteByte(&eeprom_i2c, EEPROM_ADDR);
    if (ok != 0U)
    {
        ok = SW_I2C_WriteByte(&eeprom_i2c, (uint8_t)addr);
    }
    for (i = 0U; (i < len) && (ok != 0U); i++)
    {
        ok = SW_I2C_WriteByte(&eeprom_i2c, buf[i]);
    }
    SW_I2C_Stop(&eeprom_i2c);
    return (ok != 0U) ? HAL_OK : HAL_ERROR;
}

/**
 * @brief Software I2C block read (random address).
 */
static HAL_StatusTypeDef SW_Read(uint16_t addr, uint8_t* buf, uint16_t len)
{
    uint16_t i;
    uint8_t ok;

    SW_I2C_Start(&eeprom_i2c);
    ok = SW_I2C_WriteByte(&eeprom_i2c, EEPROM_ADDR);
    if (ok != 0U)
    {
        ok = SW_I2C_WriteByte(&eeprom_i2c, (uint8_t)addr);
    }
    if (ok != 0U)
    {
        SW_I2C_Start(&eeprom_i2c); /* repeated start */
        ok = SW_I2C_WriteByte(&eeprom_i2c, EEPROM_ADDR | 1U);
    }
    for (i = 0U; (i < len) && (ok != 0U); i++)
    {
        buf[i] = SW_I2C_ReadByte(&eeprom_i2c);
        SW_I2C_Ack(&eeprom_i2c, (i == (uint16_t)(len - 1U)) ? 1U : 0U);
    }
    SW_I2C_Stop(&eeprom_i2c);
    return (ok != 0U) ? HAL_OK : HAL_ERROR;
}

void EEPROM_Init(void)
{
    SW_I2C_Init(&eeprom_i2c);
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
