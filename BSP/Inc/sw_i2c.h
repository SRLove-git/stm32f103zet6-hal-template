/**
 ******************************************************************************
 * @file    sw_i2c.h
 * @brief   Generic bit-banged (software) I2C master.
 *
 *          Shared by the 24C02 EEPROM and MPU6050 drivers: the STM32F1
 *          hardware I2C peripheral is unreliable, so a single bit-banged
 *          implementation is used instead.
 ******************************************************************************
 */

#ifndef __SW_I2C_H
#define __SW_I2C_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

    typedef struct
    {
        GPIO_TypeDef* scl_port;
        uint16_t scl_pin;
        GPIO_TypeDef* sda_port;
        uint16_t sda_pin;
    } SW_I2C_t;

    /**
     * @brief Configure both pins as open-drain outputs (needs board pull-ups).
     */
    void SW_I2C_Init(SW_I2C_t* i2c);

    void SW_I2C_Start(SW_I2C_t* i2c);
    void SW_I2C_Stop(SW_I2C_t* i2c);

    /**
     * @brief Write one byte (MSB first).
     * @retval 1 if the slave ACKed, 0 otherwise.
     */
    uint8_t SW_I2C_WriteByte(SW_I2C_t* i2c, uint8_t byte);

    /**
     * @brief Read one byte.
     */
    uint8_t SW_I2C_ReadByte(SW_I2C_t* i2c);

    /**
     * @brief Send ACK (0) or NACK (1) after reading a byte.
     */
    void SW_I2C_Ack(SW_I2C_t* i2c, uint8_t nack);

#ifdef __cplusplus
}
#endif

#endif /* __SW_I2C_H */
