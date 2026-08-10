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
#include "key.h"
#include "lcd.h"
#include "led.h"
#include "lsens.h"
#include "mpu6050.h"
#include "usart.h"

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

    (void)CLI_Register(&led0_cmd);
    (void)CLI_Register(&led1_cmd);
    (void)CLI_Register(&beep_cmd);
    (void)CLI_Register(&att_cmd);
    (void)CLI_Register(&mpu_cmd);
    (void)CLI_Register(&adc_cmd);
    (void)CLI_Register(&echo_cmd);

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
