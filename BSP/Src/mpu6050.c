/**
 ******************************************************************************
 * @file    mpu6050.c
 * @brief   ATK-MPU6050 driver (bit-banged I2C on PB10/PB11).
 ******************************************************************************
 */

#include "mpu6050.h"
#include "attitude.h"
#include "sw_i2c.h"

/* Registers */
#define MPU6050_REG_SMPLRT_DIV 0x19U
#define MPU6050_REG_CONFIG 0x1AU
#define MPU6050_REG_GYRO_CONFIG 0x1BU
#define MPU6050_REG_ACCEL_CONFIG 0x1CU
#define MPU6050_REG_ACCEL_XOUT_H 0x3BU
#define MPU6050_REG_PWR_MGMT_1 0x6BU
#define MPU6050_REG_WHO_AM_I 0x75U

/* Scale factors for the configured ranges (+-16 g, +-2000 dps) */
#define MPU6050_ACCEL_SCALE 0.00048828125f /* 16.0f / 32768 */
#define MPU6050_GYRO_SCALE 0.06103515625f  /* 2000.0f / 32768 */

/* Two pin orders exist on ATK-socket MPU modules (see mpu6050.h):
 *   A: VCC GND SCL SDA ... (SCL = PB11, SDA = PB10) - default
 *   B: official ATK module (VCC GND SDA SCL INT AD0) - SCL = PB10, SDA = PB11 */
#define MPU_PIN_ORDER_A_SCL GPIO_PIN_11
#define MPU_PIN_ORDER_A_SDA GPIO_PIN_10
#define MPU_PIN_ORDER_B_SCL GPIO_PIN_10
#define MPU_PIN_ORDER_B_SDA GPIO_PIN_11

/* Detected slave address (0x68 or 0x69 depending on AD0) */
static uint8_t mpu_addr = MPU6050_ADDR;

/* Detected pin order: 0 = A (SCL=PB11), 1 = B (SCL=PB10) */
static uint8_t mpu_pin_order = 0U;

static SW_I2C_t mpu_i2c;

/* Apply the detected pin order (A: SCL=PB11/SDA=PB10, B: swapped). */
static void MPU_ApplyPinOrder(void)
{
    if (mpu_pin_order == 0U)
    {
        mpu_i2c.scl_port = GPIOB;
        mpu_i2c.scl_pin = MPU_PIN_ORDER_A_SCL;
        mpu_i2c.sda_port = GPIOB;
        mpu_i2c.sda_pin = MPU_PIN_ORDER_A_SDA;
    }
    else
    {
        mpu_i2c.scl_port = GPIOB;
        mpu_i2c.scl_pin = MPU_PIN_ORDER_B_SCL;
        mpu_i2c.sda_port = GPIOB;
        mpu_i2c.sda_pin = MPU_PIN_ORDER_B_SDA;
    }
}

/**
 * @brief Write one register.
 */
static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t ok;

    SW_I2C_Start(&mpu_i2c);
    ok = SW_I2C_WriteByte(&mpu_i2c, (uint8_t)(mpu_addr << 1U));
    if (ok != 0U)
    {
        ok = SW_I2C_WriteByte(&mpu_i2c, reg);
    }
    if (ok != 0U)
    {
        ok = SW_I2C_WriteByte(&mpu_i2c, value);
    }
    SW_I2C_Stop(&mpu_i2c);
    return ok;
}

/**
 * @brief Read one register.
 */
static uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t value = 0xFFU;

    SW_I2C_Start(&mpu_i2c);
    if (SW_I2C_WriteByte(&mpu_i2c, (uint8_t)(mpu_addr << 1U)) != 0U)
    {
        if (SW_I2C_WriteByte(&mpu_i2c, reg) != 0U)
        {
            SW_I2C_Start(&mpu_i2c);
            if (SW_I2C_WriteByte(&mpu_i2c, (uint8_t)((mpu_addr << 1U) | 1U)) != 0U)
            {
                value = SW_I2C_ReadByte(&mpu_i2c);
                SW_I2C_Ack(&mpu_i2c, 1U); /* NACK: last byte */
            }
        }
    }
    SW_I2C_Stop(&mpu_i2c);
    return value;
}

/**
 * @brief Read a block of registers (burst read).
 */
static uint8_t MPU6050_ReadBuf(uint8_t reg, uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint8_t ok = 0U;

    SW_I2C_Start(&mpu_i2c);
    if (SW_I2C_WriteByte(&mpu_i2c, (uint8_t)(mpu_addr << 1U)) != 0U)
    {
        if (SW_I2C_WriteByte(&mpu_i2c, reg) != 0U)
        {
            SW_I2C_Start(&mpu_i2c);
            if (SW_I2C_WriteByte(&mpu_i2c, (uint8_t)((mpu_addr << 1U) | 1U)) != 0U)
            {
                for (i = 0U; i < len; i++)
                {
                    buf[i] = SW_I2C_ReadByte(&mpu_i2c);
                    SW_I2C_Ack(&mpu_i2c, (i == (uint8_t)(len - 1U)) ? 1U : 0U);
                }
                ok = 1U;
            }
        }
    }
    SW_I2C_Stop(&mpu_i2c);
    return ok;
}

uint8_t MPU6050_ReadID(void)
{
    return MPU6050_ReadReg(MPU6050_REG_WHO_AM_I);
}

static uint8_t MPU6050_IsValidID(uint8_t id)
{
    return (id == 0x68U) || (id == 0x69U) || (id == 0x70U);
}

uint8_t MPU6050_Init(void)
{
    uint8_t order;
    uint8_t addr_idx;
    uint8_t found = 0U;
    uint8_t retry;

    /* Set the default pin order first, then configure the GPIOs. */
    mpu_pin_order = 0U;
    MPU_ApplyPinOrder();
    SW_I2C_Init(&mpu_i2c);

    /* Probe both pin orders x both slave addresses to find the module */
    for (order = 0U; (order < 2U) && (found == 0U); order++)
    {
        mpu_pin_order = order;
        MPU_ApplyPinOrder();
        for (addr_idx = 0U; (addr_idx < 2U) && (found == 0U); addr_idx++)
        {
            mpu_addr = (addr_idx == 0U) ? 0x68U : 0x69U;
            if (MPU6050_IsValidID(MPU6050_ReadID()) != 0U)
            {
                found = 1U;
            }
        }
    }

    if (found == 0U)
    {
        mpu_pin_order = 0U;
        mpu_addr = MPU6050_ADDR;
        return 1U; /* module not present */
    }

    /* Reset + configure, then verify by read-back; retry on transient
     * I2C glitches (e.g. the module still settling after power-on). */
    for (retry = 0U; retry < 3U; retry++)
    {
        (void)MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80U); /* reset */
        HAL_Delay(100U);
        (void)MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01U);
        (void)MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x07U);   /* 1 kHz / 8 = 125 Hz */
        (void)MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x06U);       /* DLPF 5 Hz */
        (void)MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x18U);  /* +-2000 dps */
        (void)MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x18U); /* +-16 g */

        if ((MPU6050_ReadReg(MPU6050_REG_GYRO_CONFIG) == 0x18U) &&
            (MPU6050_ReadReg(MPU6050_REG_ACCEL_CONFIG) == 0x18U) &&
            (MPU6050_ReadReg(MPU6050_REG_PWR_MGMT_1) == 0x01U))
        {
            return 0U;
        }
    }

    return 1U; /* configuration could not be verified */
}

void MPU6050_ReadRaw(int16_t accel[3], int16_t gyro[3], int16_t* temp)
{
    uint8_t buf[14];
    uint8_t i;

    if (MPU6050_ReadBuf(MPU6050_REG_ACCEL_XOUT_H, buf, 14U) == 0U)
    {
        for (i = 0U; i < 3U; i++)
        {
            accel[i] = 0;
            gyro[i] = 0;
        }
        *temp = 0;
        return;
    }

    for (i = 0U; i < 3U; i++)
    {
        accel[i] = (int16_t)(((uint16_t)buf[i * 2U] << 8U) | buf[i * 2U + 1U]);
    }
    *temp = (int16_t)(((uint16_t)buf[6U] << 8U) | buf[7U]);
    for (i = 0U; i < 3U; i++)
    {
        gyro[i] = (int16_t)(((uint16_t)buf[8U + i * 2U] << 8U) | buf[9U + i * 2U]);
    }
}

void MPU6050_ReadAccelG(float accel_g[3])
{
    int16_t raw[3];
    int16_t gyro[3];
    int16_t temp;
    uint8_t i;

    MPU6050_ReadRaw(raw, gyro, &temp);
    for (i = 0U; i < 3U; i++)
    {
        accel_g[i] = (float)raw[i] * MPU6050_ACCEL_SCALE;
    }
}

void MPU6050_ReadGyroDps(float gyro_dps[3])
{
    int16_t gyro[3];
    int16_t accel[3];
    int16_t temp;
    uint8_t i;

    MPU6050_ReadRaw(accel, gyro, &temp);
    for (i = 0U; i < 3U; i++)
    {
        gyro_dps[i] = (float)gyro[i] * MPU6050_GYRO_SCALE;
    }
}

float MPU6050_ReadTempC(void)
{
    int16_t accel[3];
    int16_t gyro[3];
    int16_t temp;

    MPU6050_ReadRaw(accel, gyro, &temp);
    return (float)temp / 340.0f + 36.53f;
}

void MPU6050_GetAttitude(float euler[3])
{
    static uint32_t last_tick = 0U;
    uint32_t now;
    float dt;
    float accel_g[3];
    float gyro_dps[3];

    now = HAL_GetTick();
    dt = (float)(now - last_tick) / 1000.0f;
    if ((dt <= 0.0f) || (dt > 0.1f))
    {
        dt = 0.01f; /* first call or long gap: use a safe default */
    }
    last_tick = now;

    MPU6050_ReadAccelG(accel_g);
    MPU6050_ReadGyroDps(gyro_dps);

    ATT_UpdateIMU(gyro_dps[0] * ATT_DEG2RAD, gyro_dps[1] * ATT_DEG2RAD, gyro_dps[2] * ATT_DEG2RAD,
                  accel_g[0], accel_g[1], accel_g[2], dt);
    ATT_GetEuler(&euler[0], &euler[1], &euler[2]);
}
