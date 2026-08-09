/**
 ******************************************************************************
 * @file    mpu6050.h
 * @brief   ATK-MPU6050 six-axis sensor driver (ATK module interface).
 *
 *          On the Elite board the module plugs into the ATK MODULE socket.
 *          Measured on hardware (silkscreen order differs from the manual):
 *            IIC_SCL - PB11 (GBC_TX) | IIC_SDA - PB10 (GBC_RX)
 *            MPU_INT - PA4  (GBC_KEY) | MPU_AD0 - PA15 (GBC_LED, optional)
 *
 *          The module has 4.7k I2C pull-ups and a 10k AD0 pull-down, so the
 *          slave address is 0x68 by default. The chip reports WHO_AM_I 0x68
 *          (MPU6050), 0x69 (AD0 high) or 0x70 (MPU6500-compatible), all with
 *          the same register map for accel/gyro/temperature.
 *          Communication uses bit-banged I2C (software), as STM32F1
 *          hardware I2C is notoriously quirky.
 ******************************************************************************
 */

#ifndef __MPU6050_H
#define __MPU6050_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define MPU6050_ADDR 0x68U /* I2C address with AD0 low (default) */

    /**
     * @brief Initialize GPIO + software I2C and configure the sensor.
     * @retval 0 on success, 1 if the module is not detected (WHO_AM_I != 0x68).
     */
    uint8_t MPU6050_Init(void);

    /**
     * @brief Read the WHO_AM_I register (should be 0x68).
     */
    uint8_t MPU6050_ReadID(void);

    /**
     * @brief Read raw accel/gyro/temperature data.
     * @param accel  [out] X/Y/Z acceleration (LSB, range +-16 g)
     * @param gyro   [out] X/Y/Z angular rate (LSB, range +-2000 dps)
     * @param temp   [out] raw temperature (LSB)
     */
    void MPU6050_ReadRaw(int16_t accel[3], int16_t gyro[3], int16_t* temp);

    /**
     * @brief Read acceleration in g.
     */
    void MPU6050_ReadAccelG(float accel_g[3]);

    /**
     * @brief Read angular rate in degrees/second.
     */
    void MPU6050_ReadGyroDps(float gyro_dps[3]);

    /**
     * @brief Read temperature in degrees Celsius.
     */
    float MPU6050_ReadTempC(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_H */
