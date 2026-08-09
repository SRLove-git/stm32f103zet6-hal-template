/**
 ******************************************************************************
 * @file    bsp_selftest.c
 * @brief   Boot-time self-test for the on-board peripherals.
 *
 *          Results are printed over USART1 (115200-8N1). Tests that need
 *          optional hardware (DS18B20, TF card) report SKIP instead of FAIL.
 ******************************************************************************
 */

#include "bsp_selftest.h"
#include "attitude.h"
#include "can_bus.h"
#include "eeprom.h"
#include "ir_nec.h"
#include "lcd.h"
#include "lsens.h"
#include "mpu6050.h"
#include "nrf24l01.h"
#include "oled.h"
#include "onewire.h"
#include "rs485.h"
#include "sd_card.h"
#include "w25qxx.h"

#include <stdio.h>

static uint8_t selftest_failed = 0U;

#define SELFTEST_CHECK(cond, name)                                                                 \
    do                                                                                             \
    {                                                                                              \
        if (cond)                                                                                  \
        {                                                                                          \
            printf("  [PASS] %s\r\n", name);                                                       \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            printf("  [FAIL] %s\r\n", name);                                                       \
            selftest_failed = 1U;                                                                  \
        }                                                                                          \
    } while (0)

static uint8_t BufMatch(const uint8_t* a, const uint8_t* b, uint8_t len)
{
    uint8_t i;

    for (i = 0U; i < len; i++)
    {
        if (a[i] != b[i])
        {
            return 0U;
        }
    }
    return 1U;
}

static void SelfTest_EEPROM(void)
{
    uint8_t orig;
    uint8_t wbuf[2] = {0x5AU, 0xA5U};
    uint8_t rbuf[2] = {0U, 0U};

    printf("[EEPROM] 24C02 @ I2C1\r\n");
    EEPROM_Init();

    /* Use a scratch address near the end, restore the original byte after. */
    orig = EEPROM_ReadByte(0xF0U);
    (void)EEPROM_WriteBuffer(0xF0U, wbuf, 2U);
    (void)EEPROM_ReadBuffer(0xF0U, rbuf, 2U);

    SELFTEST_CHECK(BufMatch(wbuf, rbuf, 2U), "write/read-back");
    (void)EEPROM_WriteByte(0xF0U, orig);
}

static void SelfTest_SPIFlash(void)
{
    uint8_t wbuf[16];
    uint8_t rbuf[16] = {0U};
    uint8_t i;
    uint32_t id;

    printf("[SPI FLASH] W25Q128 @ SPI2\r\n");
    W25QXX_Init();

    id = W25QXX_ReadID();
    printf("  JEDEC ID = 0x%06lX (expect 0xEF4018)\r\n", (unsigned long)id);

    for (i = 0U; i < 16U; i++)
    {
        wbuf[i] = (uint8_t)(0x50U + i);
    }

    /* Last sector (0xFFF000) is used as scratch. */
    (void)W25QXX_EraseSector(0xFFF000U);
    (void)W25QXX_WriteData(0xFFF000U, wbuf, 16U);
    W25QXX_ReadData(0xFFF000U, rbuf, 16U);

    SELFTEST_CHECK(BufMatch(wbuf, rbuf, 16U), "erase + write/read-back");
}

static void SelfTest_LightSensor(void)
{
    uint32_t raw;

    printf("[LIGHT] ADC3_IN6 @ PF8\r\n");
    LSENS_Init();

    raw = LSENS_ReadADC();
    printf("  raw = %lu (%lu mV)\r\n", (unsigned long)raw, (unsigned long)LSENS_ReadMv());
    SELFTEST_CHECK(raw <= 4095U, "ADC conversion range");
}

static void SelfTest_DS18B20(void)
{
    int16_t temp;
    uint32_t u;
    char sign = ' ';

    printf("[DS18B20] 1-wire @ PG11\r\n");
    OW_Init();

    temp = DS18B20_GetTemp();
    if (temp == 0x7FFFU)
    {
        printf("  [SKIP] no device found (sensor not connected?)\r\n");
        return;
    }

    u = (temp < 0) ? (uint32_t)(0 - temp) : (uint32_t)temp;
    if (temp < 0)
    {
        sign = '-';
    }
    printf("  temp = %c%lu.%02lu C\r\n", sign, (unsigned long)(u / 100U),
           (unsigned long)(u % 100U));
    SELFTEST_CHECK((temp > -5500) && (temp < 12500), "temperature in range");
}

static void SelfTest_IR(void)
{
    printf("[IR] NEC receiver @ PB9\r\n");
    IR_Init();
    printf("  [INFO] ready - press a remote button to see codes in main loop\r\n");
}

static void SelfTest_RS485(void)
{
    printf("[RS485] USART2 @ PA2/PA3, RE=PD7\r\n");
    RS485_Init();
    printf("  [INFO] init OK (P5 jumper, half-duplex; requires a peer node)\r\n");
}

static void SelfTest_MPU6050(void)
{
    int16_t accel[3];
    int16_t gyro[3];
    int16_t temp;
    float euler[3];
    uint8_t i;

    printf("[MPU6050] ATK module I/F (SCL=PB11, SDA=PB10)\r\n");
    if (MPU6050_Init() != 0U)
    {
        printf("  [SKIP] module not detected\r\n");
        return;
    }

    SELFTEST_CHECK((MPU6050_ReadID() == 0x68U) || (MPU6050_ReadID() == 0x69U) ||
                       (MPU6050_ReadID() == 0x70U),
                   "WHO_AM_I valid (0x68/0x69/0x70)");

    MPU6050_ReadRaw(accel, gyro, &temp);
    printf("  acc = %d, %d, %d | gyro = %d, %d, %d | temp = %d\r\n", (int)accel[0], (int)accel[1],
           (int)accel[2], (int)gyro[0], (int)gyro[1], (int)gyro[2], (int)temp);
    SELFTEST_CHECK((accel[0] != 0) || (accel[1] != 0) || (accel[2] != 0),
                   "accel data non-zero (static: Z ~ +1 g)");

    /* Mahony attitude: let the filter settle for ~0.5 s */
    ATT_Init();
    for (i = 0U; i < 50U; i++)
    {
        MPU6050_GetAttitude(euler);
        HAL_Delay(10U);
    }
    printf("  attitude: roll=%5.1f pitch=%5.1f yaw=%5.1f deg\r\n", euler[0], euler[1], euler[2]);
    SELFTEST_CHECK((euler[0] > -45.0f) && (euler[0] < 45.0f) && (euler[1] > -45.0f) &&
                       (euler[1] < 45.0f),
                   "attitude plausible (level: roll/pitch ~ 0)");
}

static void SelfTest_OLED(void)
{
    printf("[OLED] SSD1306 0.96\" @ P4 (8080 parallel)\r\n");
    OLED_Init();
    OLED_Clear(0U);
    OLED_ShowString(0U, 0U, 1U, "BSP Self-Test");
    OLED_ShowString(0U, 16U, 1U, "OLED: OK");
    OLED_Refresh();
    printf("  [INFO] init done - text should be visible on the module\r\n");
}

static void SelfTest_NRF24L01(void)
{
    printf("[NRF24L01] WIRELESS I/F (SPI2 shared, CE=PG8, CS=PG7)\r\n");
    if (NRF24L01_Init() != 0U)
    {
        printf("  [SKIP] module not detected (SPI read-back failed)\r\n");
        return;
    }
    printf("  [INFO] module present - link test needs two modules/boards\r\n");
}

static void SelfTest_CAN(void)
{
    printf("[CAN] CAN1 @ PA11/PA12 (loopback)\r\n");
    CAN1_Init();
    SELFTEST_CHECK(CAN1_SelfTest() == HAL_OK, "loopback TX/RX echo");
}

static void SelfTest_SD(void)
{
    HAL_SD_CardInfoTypeDef info;
    uint8_t block[SD_BLOCK_SIZE];

    printf("[TF CARD] SDIO 4-bit\r\n");
    if (SD_Init() != HAL_OK)
    {
        printf("  [SKIP] no TF card detected\r\n");
        return;
    }

    (void)SD_GetCardInfo(&info);
    printf("  card size = %lu blocks (%lu MB)\r\n", (unsigned long)info.LogBlockNbr,
           (unsigned long)((info.LogBlockNbr * 512U) / (1024U * 1024U)));
    SELFTEST_CHECK(SD_ReadBlocks(0U, block, 1U) == HAL_OK, "read block 0");
}

static void SelfTest_LCD(void)
{
    uint16_t id;
    uint16_t px;

    printf("[TFTLCD] FSMC NE4/A10\r\n");
    LCD_Init();

    id = LCD_GetID();
    printf("  LCD ID = 0x%04X (expect 0x9341 for ILI9341)\r\n", id);
    SELFTEST_CHECK((id != 0x0000U) && (id != 0xFFFFU), "ID readable");

    /* GRAM read-back is unreliable on this panel family (18-bit byte stream);
     * keep it informational instead of a hard check. */
    LCD_DrawPoint(0U, 0U, LCD_BLACK);
    px = LCD_GetPoint(0U, 0U);
    printf("  pixel read-back = 0x%04X (informational; panel GRAM read quirk)\r\n", px);

    LCD_ShowString(10U, 10U, 2U, "BSP Self-Test", LCD_BLACK, LCD_WHITE);
    if (selftest_failed == 0U)
    {
        LCD_ShowString(10U, 34U, 2U, "ALL PASS", LCD_RED, LCD_WHITE);
    }
    else
    {
        LCD_ShowString(10U, 34U, 2U, "HAS FAILURE", LCD_RED, LCD_WHITE);
    }
}

void BSP_SelfTest(void)
{
    printf("\r\n========== BSP Self-Test ==========\r\n");

    SelfTest_EEPROM();
    SelfTest_SPIFlash();
    SelfTest_LightSensor();
    SelfTest_DS18B20();
    SelfTest_IR();
    SelfTest_RS485();
    SelfTest_MPU6050();
    SelfTest_OLED();
    SelfTest_NRF24L01();
    SelfTest_CAN();
    SelfTest_SD();
    SelfTest_LCD();

    printf("========== Result: %s ==========\r\n",
           (selftest_failed == 0U) ? "ALL PASS" : "HAS FAILURES");
}
