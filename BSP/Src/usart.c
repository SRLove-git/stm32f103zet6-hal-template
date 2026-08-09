/**
 ******************************************************************************
 * @file    usart.c
 * @brief   USART1 driver + printf retarget.
 ******************************************************************************
 */

#include "usart.h"

#include <stdio.h>

UART_HandleTypeDef huart1;

static uint8_t usart1_ready = 0U;

void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }

    usart1_ready = 1U;
}

/**
 * @brief Retarget printf()/puts() output to USART1 (newlib-nano).
 *
 *        Uses blocking polling TX; fine for debug output.
 */
int _write(int fd, char* ptr, int len)
{
    (void)fd;

    if (!usart1_ready || (len <= 0))
    {
        return len;
    }

    if (HAL_UART_Transmit(&huart1, (uint8_t*)ptr, (uint16_t)len, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    return len;
}
