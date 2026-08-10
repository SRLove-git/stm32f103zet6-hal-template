/**
 ******************************************************************************
 * @file    demo.c
 * @brief   Shared key/LCD-attitude demo (bare-metal and FreeRTOS).
 ******************************************************************************
 */

#include "demo.h"
#include "attitude.h"
#include "beep.h"
#include "cli.h"
#include "eeprom.h"
#include "key.h"
#include "lcd.h"
#include "led.h"
#include "lsens.h"
#include "mpu6050.h"
#include "settings.h"
#include "usart.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uint8_t demo_active = 0U;
static uint8_t use_madgwick = 0U;

/* Print a float as "X.Y" using integer math only (newlib %f is unreliable). */
static void PrintAngle(float deg)
{
    int32_t v10 = (int32_t)(deg * 10.0f);
    int32_t ip = v10 / 10;
    int32_t fp = v10 % 10;

    if (fp < 0)
    {
        fp = -fp;
    }
    printf("%ld.%ld", (long)ip, (long)fp);
}

static void Cmd_Led0(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("usage: led0 on|off|toggle\r\n");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        LED0_On();
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        LED0_Off();
    }
    else if (strcmp(argv[1], "toggle") == 0)
    {
        LED0_Toggle();
    }
    else
    {
        printf("usage: led0 on|off|toggle\r\n");
    }
}

static void Cmd_Led1(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("usage: led1 on|off|toggle\r\n");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        LED1_On();
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        LED1_Off();
    }
    else if (strcmp(argv[1], "toggle") == 0)
    {
        LED1_Toggle();
    }
    else
    {
        printf("usage: led1 on|off|toggle\r\n");
    }
}

static void Cmd_Beep(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("usage: beep on|off|toggle\r\n");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        BEEP_On();
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        BEEP_Off();
    }
    else if (strcmp(argv[1], "toggle") == 0)
    {
        BEEP_Toggle();
    }
    else
    {
        printf("usage: beep on|off|toggle\r\n");
    }
}

static void Cmd_Att(int argc, char* argv[])
{
    float euler[3];

    (void)argc;
    (void)argv;

    if (demo_active == 0U)
    {
        printf("MPU6050 not present\r\n");
        return;
    }
    MPU6050_GetAttitude(euler);
    printf("roll=");
    PrintAngle(euler[0]);
    printf(" pitch=");
    PrintAngle(euler[1]);
    printf(" yaw=");
    PrintAngle(euler[2]);
    printf(" deg (%s)\r\n", use_madgwick != 0U ? "Madgwick" : "Mahony");
}

static void Cmd_Mpu(int argc, char* argv[])
{
    int16_t accel[3];
    int16_t gyro[3];
    int16_t temp;

    (void)argc;
    (void)argv;

    if (demo_active == 0U)
    {
        printf("MPU6050 not present\r\n");
        return;
    }
    MPU6050_ReadRaw(accel, gyro, &temp);
    printf("acc=%d,%d,%d gyro=%d,%d,%d temp=%d\r\n", (int)accel[0], (int)accel[1], (int)accel[2],
           (int)gyro[0], (int)gyro[1], (int)gyro[2], (int)temp);
}

static void Cmd_Adc(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    LSENS_Init();
    printf("light raw=%lu (%lu mV)\r\n", (unsigned long)LSENS_ReadADC(),
           (unsigned long)LSENS_ReadMv());
}

static void Cmd_Echo(int argc, char* argv[])
{
    int i;

    for (i = 1; i < argc; i++)
    {
        printf("%s%s", argv[i], (i < argc - 1) ? " " : "");
    }
    printf("\r\n");
}

static void Cmd_Settings(int argc, char* argv[])
{
    uint32_t val;
    uint8_t raw[16];
    uint8_t i;

    if (argc == 1)
    {
        printf("settings: pwm_max=%u alpha_x10=%u flags=0x%02X\r\n",
               (unsigned)g_settings.motor_pwm_max, (unsigned)g_settings.filter_alpha_x10,
               (unsigned)g_settings.flags);
        return;
    }

    if ((strcmp(argv[1], "dump") == 0) && (argc == 2))
    {
        if (EEPROM_ReadBuffer(SETTINGS_EEPROM_ADDR, raw, sizeof(raw)) != HAL_OK)
        {
            printf("eeprom read failed\r\n");
            return;
        }
        for (i = 0U; i < 16U; i++)
        {
            printf("%02X%s", raw[i], (i == 15U) ? "\r\n" : " ");
        }
        return;
    }

    if ((strcmp(argv[1], "reset") == 0) && (argc == 2))
    {
        SETTINGS_Reset();
        printf("settings reset to defaults\r\n");
        return;
    }

    if ((argc < 4) || (strcmp(argv[1], "set") != 0))
    {
        printf("usage: settings | settings reset | settings set <field> <value>\r\n");
        return;
    }

    val = strtoul(argv[3], NULL, 0);
    if (strcmp(argv[2], "pwm_max") == 0)
    {
        g_settings.motor_pwm_max = (uint16_t)val;
    }
    else if (strcmp(argv[2], "alpha") == 0)
    {
        g_settings.filter_alpha_x10 = (uint8_t)val;
    }
    else if (strcmp(argv[2], "flags") == 0)
    {
        g_settings.flags = (uint8_t)val;
    }
    else
    {
        printf("unknown field: %s\r\n", argv[2]);
        return;
    }

    printf("saved: %s=%lu (%s)\r\n", argv[2], (unsigned long)val,
           (SETTINGS_Save() == HAL_OK) ? "i2c ok" : "i2c ERR");
}

static void Cmd_EepromTest(int argc, char* argv[])
{
    uint8_t wbuf[2] = {0x5AU, 0xA5U};
    uint8_t rbuf[2] = {0U, 0U};
    uint8_t orig;
    HAL_StatusTypeDef wr;
    HAL_StatusTypeDef rd;

    (void)argc;
    (void)argv;

    EEPROM_Init();
    orig = EEPROM_ReadByte(0xF0U);
    wr = EEPROM_WriteBuffer(0xF0U, wbuf, 2U);
    rd = EEPROM_ReadBuffer(0xF0U, rbuf, 2U);
    printf("eeprom test: %s (orig 0x%02X, wr=%d rd=%d)\r\n",
           ((rbuf[0] == 0x5AU) && (rbuf[1] == 0xA5U)) ? "PASS" : "FAIL", orig, (int)wr, (int)rd);
    (void)EEPROM_WriteByte(0xF0U, orig);
}

/**
 * @brief Show one angle ("-123.4") at fixed width using integer math
 *        (the bundled newlib %f is unreliable, see README).
 */
static void LCD_ShowAngle(uint16_t x, uint16_t y, float deg, uint8_t scale, uint16_t color,
                          uint16_t bg)
{
    int32_t v10 = (int32_t)(deg * 10.0f);
    int32_t ip = v10 / 10;
    int32_t fp = v10 % 10;

    if (fp < 0)
    {
        fp = -fp;
    }

    LCD_ShowChar(x, y, (ip < 0) ? '-' : ' ', scale, color, bg);
    if (ip < 0)
    {
        ip = -ip;
    }
    x = (uint16_t)(x + 6U * scale);

    LCD_ShowNum(x, y, (uint32_t)ip, 3U, scale, color, bg);
    x = (uint16_t)(x + 18U * scale);

    LCD_ShowChar(x, y, '.', scale, color, bg);
    x = (uint16_t)(x + 6U * scale);

    LCD_ShowNum(x, y, (uint32_t)fp, 1U, scale, color, bg);
}

uint8_t Demo_Init(void)
{
    static const CLI_Cmd_t led0_cmd = {"led0", "led0 on|off|toggle", Cmd_Led0};
    static const CLI_Cmd_t led1_cmd = {"led1", "led1 on|off|toggle", Cmd_Led1};
    static const CLI_Cmd_t beep_cmd = {"beep", "beep on|off|toggle", Cmd_Beep};
    static const CLI_Cmd_t att_cmd = {"att", "show attitude angles", Cmd_Att};
    static const CLI_Cmd_t mpu_cmd = {"mpu", "show raw MPU6050 data", Cmd_Mpu};
    static const CLI_Cmd_t adc_cmd = {"adc", "show light sensor ADC", Cmd_Adc};
    static const CLI_Cmd_t echo_cmd = {"echo", "echo arguments back", Cmd_Echo};
    static const CLI_Cmd_t settings_cmd = {"settings", "show/set persistent settings",
                                           Cmd_Settings};
    static const CLI_Cmd_t eeprom_cmd = {"eeprom", "run 24C02 write/read-back test",
                                         Cmd_EepromTest};

    (void)CLI_Register(&led0_cmd);
    (void)CLI_Register(&led1_cmd);
    (void)CLI_Register(&beep_cmd);
    (void)CLI_Register(&att_cmd);
    (void)CLI_Register(&mpu_cmd);
    (void)CLI_Register(&adc_cmd);
    (void)CLI_Register(&echo_cmd);
    (void)CLI_Register(&settings_cmd);
    (void)CLI_Register(&eeprom_cmd);

    SETTINGS_Load();

    demo_active = (MPU6050_Init() == 0U);
    use_madgwick = 0U;

    if (demo_active != 0U)
    {
        LCD_Init();
        ATT_SetFilter(ATT_FILTER_MAHONY);
        ATT_Init();

        LCD_Clear(LCD_WHITE);
        LCD_ShowString(10U, 10U, 2U, "Attitude (Mahony)", LCD_BLACK, LCD_WHITE);
        LCD_ShowString(10U, 40U, 2U, "KEY_UP: switch filter", LCD_BLUE, LCD_WHITE);
        LCD_ShowString(10U, 60U, 2U, "Roll :", LCD_BLACK, LCD_WHITE);
        LCD_ShowString(10U, 130U, 2U, "Pitch:", LCD_BLACK, LCD_WHITE);
        LCD_ShowString(10U, 200U, 2U, "Yaw  :", LCD_BLACK, LCD_WHITE);
    }

    return demo_active;
}

void Demo_KeyScan(void)
{
    uint8_t key = KEY_Scan();

    switch (key)
    {
        case KEY0_PRESS:
            printf("KEY0 pressed\r\n");
            BEEP_On();
            HAL_Delay(200);
            BEEP_Off();
            break;

        case KEY1_PRESS:
            printf("KEY1 pressed\r\n");
            LED0_Toggle();
            break;

        case KEY_UP_PRESS:
            if (demo_active != 0U)
            {
                use_madgwick ^= 1U;
                ATT_SetFilter(use_madgwick != 0U ? ATT_FILTER_MADGWICK : ATT_FILTER_MAHONY);
                ATT_Init();
                LCD_Fill(10U, 10U, 230U, 30U, LCD_WHITE);
                LCD_ShowString(10U, 10U, 2U,
                               use_madgwick != 0U ? "Attitude (Madgwick)" : "Attitude (Mahony)",
                               LCD_BLACK, LCD_WHITE);
            }
            else
            {
                printf("KEY_UP pressed\r\n");
                LED1_Toggle();
            }
            break;

        default:
            break;
    }
}

void Demo_AttitudeUpdate(void)
{
    float euler[3];

    if (demo_active == 0U)
    {
        LED0_Toggle(); /* heartbeat when no MPU */
        return;
    }

    MPU6050_GetAttitude(euler);
    LCD_ShowAngle(80U, 60U, euler[0], 3U, LCD_BLACK, LCD_WHITE);
    LCD_ShowAngle(80U, 130U, euler[1], 3U, LCD_BLACK, LCD_WHITE);
    LCD_ShowAngle(80U, 200U, euler[2], 3U, LCD_BLACK, LCD_WHITE);
}

void Demo_CliPoll(void)
{
    uint8_t ch;

    while (USART1_RxGet(&ch) != 0U)
    {
        CLI_Feed(ch);
    }
}
