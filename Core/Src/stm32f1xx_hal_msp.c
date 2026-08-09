/**
  ******************************************************************************
  * @file    stm32f1xx_hal_msp.c
  * @brief   HAL MSP module (peripheral low-level initialization).
  *
  *          Follows the STM32CubeMX convention: MSP = MCU Support Package,
  *          i.e. clocks, pins and NVIC belonging to each peripheral.
  ******************************************************************************
  */

#include "main.h"
#include "usart.h"

/**
  * @brief Initializes the global MSP.
  *        (AFIO/PWR clocks, interrupt priority grouping, SysTick priority)
  */
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* 4 bits preemption priority, 0 bits subpriority (Cortex-M3 default) */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * @brief USART MSP Initialization.
  *        USART1: PA9 (TX) / PA10 (RX), 115200-8N1, connected to CH340C
  *        through the P3 jumpers on the Elite board.
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* USART1 GPIO Configuration: PA9 -> TX (AF push-pull),
         *                            PA10 -> RX (floating input) */
        GPIO_InitStruct.Pin       = GPIO_PIN_9;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* SysTick keeps priority 0; USART1 uses priority 1 */
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
  * @brief USART MSP De-Initialization.
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_DISABLE();

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}
