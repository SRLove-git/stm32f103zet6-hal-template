/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32F103ZET6 HAL template - main program.
 *
 *          Board : ALIENTEK Elite (ATK-DNF103) STM32F103ZET6
 *          Clock : HSE 8 MHz, PLL x9 -> SYSCLK 72 MHz
 *
 *          Demo  : BSP self-test, then key + LCD attitude display.
 *                  Build with -DUSE_FREERTOS=ON for the task-based variant
 *                  (see Core/Src/freertos.c).
 ******************************************************************************
 */

#include "main.h"
#include "beep.h"
#include "bsp_selftest.h"
#include "demo.h"
#include "freertos_app.h"
#include "key.h"
#include "led.h"
#include "usart.h"

#include <stdio.h>

static void MX_GPIO_Init(void);

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

#ifdef USE_FREERTOS
    /* Task-based demo: attitude LCD task + key task. Never returns. */
    App_FreeRTOS_Init();
#else
    /* Bare-metal demo loop (10 Hz): keys + LCD attitude refresh. */
    (void)Demo_Init();
    while (1)
    {
        Demo_KeyScan();
        Demo_AttitudeUpdate();
        HAL_Delay(100);
    }
#endif
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
