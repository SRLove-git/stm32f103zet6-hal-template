/**
 ******************************************************************************
 * @file    mpu6050.c
 * @brief   ATK-MPU6050 driver (bit-banged I2C on PB10/PB11).
 ******************************************************************************
 */

#include "mpu6050.h"
#include "attitude.h"
#include "bsp_dwt.h"

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

#define MPU_SCL_PORT GPIOB
#define MPU_SDA_PORT GPIOB

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

static void MPU_I2C_Delay(void)
{
    BSP_DWT_DelayUs(5U); /* ~100 kHz */
}

static void MPU_SCL_Write(uint8_t level)
{
    uint16_t pin = (mpu_pin_order == 0U) ? MPU_PIN_ORDER_A_SCL : MPU_PIN_ORDER_B_SCL;
    HAL_GPIO_WritePin(MPU_SCL_PORT, pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void MPU_SDA_Write(uint8_t level)
{
    uint16_t pin = (mpu_pin_order == 0U) ? MPU_PIN_ORDER_A_SDA : MPU_PIN_ORDER_B_SDA;
    HAL_GPIO_WritePin(MPU_SDA_PORT, pin, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t MPU_SDA_Read(void)
{
    uint16_t pin = (mpu_pin_order == 0U) ? MPU_PIN_ORDER_A_SDA : MPU_PIN_ORDER_B_SDA;
    return (HAL_GPIO_ReadPin(MPU_SDA_PORT, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static void MPU_I2C_Start(void)
{
    MPU_SDA_Write(1U);
    MPU_SCL_Write(1U);
    MPU_I2C_Delay();
    MPU_SDA_Write(0U);
    MPU_I2C_Delay();
    MPU_SCL_Write(0U);
    MPU_I2C_Delay();
}

static void MPU_I2C_Stop(void)
{
    MPU_SDA_Write(0U);
    MPU_SCL_Write(1U);
    MPU_I2C_Delay();
    MPU_SDA_Write(1U);
    MPU_I2C_Delay();
}

static void MPU_I2C_SendByte(uint8_t byte)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        MPU_SCL_Write(0U);
        MPU_I2C_Delay();
        MPU_SDA_Write((byte & 0x80U) != 0U);
        MPU_I2C_Delay();
        MPU_SCL_Write(1U);
        MPU_I2C_Delay();
        byte <<= 1U;
    }
    MPU_SCL_Write(0U);
    MPU_I2C_Delay();
}

static uint8_t MPU_I2C_RecvByte(void)
{
    uint8_t i;
    uint8_t byte = 0U;

    MPU_SDA_Write(1U); /* release the bus */
    for (i = 0U; i < 8U; i++)
    {
        byte <<= 1U;
        MPU_SCL_Write(0U);
        MPU_I2C_Delay();
        MPU_SCL_Write(1U);
        MPU_I2C_Delay();
        if (MPU_SDA_Read() != 0U)
        {
            byte |= 0x01U;
        }
    }
    MPU_SCL_Write(0U);
    MPU_I2C_Delay();
    return byte;
}

static uint8_t MPU_I2C_WaitAck(void)
{
    uint8_t ack;

    MPU_SDA_Write(1U); /* release the bus */
    MPU_I2C_Delay();
    MPU_SCL_Write(1U);
    MPU_I2C_Delay();
    ack = (MPU_SDA_Read() == 0U) ? 1U : 0U; /* slave pulls low = ACK */
    MPU_SCL_Write(0U);
    MPU_I2C_Delay();
    return ack;
}

static void MPU_I2C_Ack(uint8_t ack)
{
    MPU_SDA_Write(ack); /* 1 = NACK (release SDA), 0 = ACK (pull low) */
    MPU_I2C_Delay();
    MPU_SCL_Write(1U);
    MPU_I2C_Delay();
    MPU_SCL_Write(0U);
    MPU_I2C_Delay();
    MPU_SDA_Write(1U);
}

/**
 * @brief Write one register.
 */
static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t ok;

    MPU_I2C_Start();
    MPU_I2C_SendByte((uint8_t)(mpu_addr << 1U));
    ok = MPU_I2C_WaitAck();
    if (ok != 0U)
    {
        MPU_I2C_SendByte(reg);
        ok = MPU_I2C_WaitAck();
    }
    if (ok != 0U)
    {
        MPU_I2C_SendByte(value);
        ok = MPU_I2C_WaitAck();
    }
    MPU_I2C_Stop();
    return ok;
}

/**
 * @brief Read one register.
 */
static uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t value = 0xFFU;

    MPU_I2C_Start();
    MPU_I2C_SendByte((uint8_t)(mpu_addr << 1U));
    if (MPU_I2C_WaitAck() != 0U)
    {
        MPU_I2C_SendByte(reg);
        if (MPU_I2C_WaitAck() != 0U)
        {
            MPU_I2C_Start();
            MPU_I2C_SendByte((uint8_t)((mpu_addr << 1U) | 1U));
            if (MPU_I2C_WaitAck() != 0U)
            {
                value = MPU_I2C_RecvByte();
                MPU_I2C_Ack(1U); /* NACK: last byte */
            }
        }
    }
    MPU_I2C_Stop();
    return value;
}

/**
 * @brief Read a block of registers (burst read).
 */
static uint8_t MPU6050_ReadBuf(uint8_t reg, uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint8_t ok = 0U;

    MPU_I2C_Start();
    MPU_I2C_SendByte((uint8_t)(mpu_addr << 1U));
    if (MPU_I2C_WaitAck() != 0U)
    {
        MPU_I2C_SendByte(reg);
        if (MPU_I2C_WaitAck() != 0U)
        {
            MPU_I2C_Start();
            MPU_I2C_SendByte((uint8_t)((mpu_addr << 1U) | 1U));
            if (MPU_I2C_WaitAck() != 0U)
            {
                for (i = 0U; i < len; i++)
                {
                    buf[i] = MPU_I2C_RecvByte();
                    MPU_I2C_Ack((i == (uint8_t)(len - 1U)) ? 1U : 0U);
                }
                ok = 1U;
            }
        }
    }
    MPU_I2C_Stop();
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
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t order;
    uint8_t addr_idx;
    uint8_t found = 0U;
    uint8_t retry;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    BSP_DWT_DelayInit();

    /* Both PB10/PB11 as open-drain outputs: the module provides 4.7k pull-ups */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    MPU_SCL_Write(1U);
    MPU_SDA_Write(1U);

    /* Probe both pin orders x both slave addresses to find the module */
    for (order = 0U; (order < 2U) && (found == 0U); order++)
    {
        mpu_pin_order = order;
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
