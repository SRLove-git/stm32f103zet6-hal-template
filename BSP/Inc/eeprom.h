/**
 ******************************************************************************
 * @file    eeprom.h
 * @brief   On-board 24C02 EEPROM driver (I2C1).
 *
 *          SCL - PB6 | SDA - PB7 | device address 0xA0 (A0~A2 grounded)
 *          Capacity: 256 bytes, page size 8 bytes.
 *
 * @note    Bit-banged (software) I2C is used - the STM32F1 hardware I2C
 *          peripheral is notoriously unreliable (see the board manual).
 ******************************************************************************
 */

#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define EEPROM_ADDR 0xA0U /* 24C02 with A0=A1=A2=0 */
#define EEPROM_SIZE 256U
#define EEPROM_PAGE_SIZE 8U
#define EEPROM_TIMEOUT 100U /* ms */

    void EEPROM_Init(void);
    HAL_StatusTypeDef EEPROM_ReadBuffer(uint16_t addr, uint8_t* buf, uint16_t len);
    HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t addr, const uint8_t* buf, uint16_t len);
    uint8_t EEPROM_ReadByte(uint16_t addr);
    HAL_StatusTypeDef EEPROM_WriteByte(uint16_t addr, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H */
