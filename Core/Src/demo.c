/**
 ******************************************************************************
 * @file    demo.c
 * @brief   Shared key/LCD-attitude demo (bare-metal and FreeRTOS).
 ******************************************************************************
 */

#include "demo.h"
#include "attitude.h"
#include "beep.h"
#include "key.h"
#include "lcd.h"
#include "led.h"
#include "mpu6050.h"

#include <stdio.h>

static uint8_t demo_active = 0U;
static uint8_t use_madgwick = 0U;

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
