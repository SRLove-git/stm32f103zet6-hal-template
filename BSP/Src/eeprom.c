/**
 ******************************************************************************
 * @file    eeprom.c
 * @brief   24C02 EEPROM driver.
 ******************************************************************************
 */

#include "eeprom.h"

I2C_HandleTypeDef hi2c1;

void EEPROM_Init(void)
{
    hi2c1.Instance = EEPROM_I2C;
    hi2c1.Init.ClockSpeed = 100000U; /* 100 kHz standard mode */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0U;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0U;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

HAL_StatusTypeDef EEPROM_ReadBuffer(uint16_t addr, uint8_t* buf, uint16_t len)
{
    if ((addr + len) > EEPROM_SIZE)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_8BIT, buf, len,
                            EEPROM_TIMEOUT);
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

        ret = HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)buf,
                                chunk, EEPROM_TIMEOUT);
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
