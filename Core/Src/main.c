/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32F103ZET6 HAL template - main program.
 *
 *          Board : ALIENTEK Elite (ATK-DNF103) STM32F103ZET6
 *          Clock : HSE 8 MHz, PLL x9 -> SYSCLK 72 MHz
 *          Demo  : LED blink + KEY scan + BEEP + USART1 printf
 ******************************************************************************
 */

#include "main.h"
#include "attitude.h"
#include "bsp_selftest.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "mpu6050.h"
#include "usart.h"

#include <stdio.h>

static void MX_GPIO_Init(void);

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

int main(void)
{
    /* Reset of all peripherals, initializes the Flash interface and the
     * Systick (HAL_Init internally calls SystemInit via startup code). */
    HAL_Init();

    /* Configure the system clock to 72 MHz. */
    SystemClock_Config();

    /* Initialize board peripherals. */
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* Boot banner over USART1 (P3 jumpers must connect USART1 <-> CH340). */
    printf("\r\n======================================\r\n");
    printf("STM32F103ZET6 HAL Template\r\n");
    printf("Board   : ALIENTEK Elite (ATK-DNF103)\r\n");
    printf("SYSCLK  : %lu Hz\r\n", (unsigned long)HAL_RCC_GetSysClockFreq());
    printf("HCLK    : %lu Hz\r\n", (unsigned long)HAL_RCC_GetHCLKFreq());
    printf("PCLK1   : %lu Hz\r\n", (unsigned long)HAL_RCC_GetPCLK1Freq());
    printf("PCLK2   : %lu Hz\r\n", (unsigned long)HAL_RCC_GetPCLK2Freq());
    printf("Press KEY0 / KEY1 / KEY_UP ...\r\n");
    printf("======================================\r\n");

    /* On-board peripheral self-test (results over USART1). */
    BSP_SelfTest();

    uint8_t key;
    uint8_t mpu_ok;
    uint8_t use_madgwick = 0U;
    float euler[3];

    mpu_ok = (MPU6050_Init() == 0U);

    if (mpu_ok != 0U)
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

    while (1)
    {
        key = KEY_Scan();
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
                if (mpu_ok != 0U)
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

        if (mpu_ok != 0U)
        {
            MPU6050_GetAttitude(euler);
            LCD_ShowAngle(80U, 60U, euler[0], 3U, LCD_BLACK, LCD_WHITE);
            LCD_ShowAngle(80U, 130U, euler[1], 3U, LCD_BLACK, LCD_WHITE);
            LCD_ShowAngle(80U, 200U, euler[2], 3U, LCD_BLACK, LCD_WHITE);
        }
        else
        {
            /* Heartbeat: LED0 (red) blinks. */
            LED0_Toggle();
        }
        HAL_Delay(100);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; /* 8 MHz * 9 = 72 MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; /* 36 MHz max for APB1 */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; /* 72 MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization
 *
 *        LED0  - PB5 (red,   active low)
 *        LED1  - PE5 (green, active low)
 *        KEY0  - PE4 (active low,  internal pull-up)
 *        KEY1  - PE3 (active low,  internal pull-up)
 *        KEY_UP- PA0 (active high, internal pull-down, WKUP)
 *        BEEP  - PB8 (active high)
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* LEDs */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = LED0_PIN;
    HAL_GPIO_Init(LED0_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED1_PIN;
    HAL_GPIO_Init(LED1_PORT, &GPIO_InitStruct);

    /* Keys */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pull = GPIO_PULLUP; /* KEY0/KEY1: low level active */
    GPIO_InitStruct.Pin = KEY0_PIN;
    HAL_GPIO_Init(KEY0_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY1_PIN;
    HAL_GPIO_Init(KEY1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pull = GPIO_PULLDOWN; /* KEY_UP: high level active */
    GPIO_InitStruct.Pin = KEY_UP_PIN;
    HAL_GPIO_Init(KEY_UP_PORT, &GPIO_InitStruct);

    /* Buzzer */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = BEEP_PIN;
    HAL_GPIO_Init(BEEP_PORT, &GPIO_InitStruct);

    /* Default states */
    HAL_GPIO_WritePin(LED0_PORT, LED0_PIN, GPIO_PIN_SET);   /* LED off */
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);   /* LED off */
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET); /* BEEP off */
}

/**
 * @brief This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief Reports the name of the source file and the source line number
 *        where the assert_param error has occurred.
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif /* USE_FULL_ASSERT */
