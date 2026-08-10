/**
 ******************************************************************************
 * @file    usart.c
 * @brief   USART1 driver + printf retarget.
 ******************************************************************************
 */

#include "usart.h"
#include "ringbuf.h"

#include <stdio.h>

UART_HandleTypeDef huart1;

static uint8_t usart1_ready = 0U;
static uint8_t usart1_rx_buf[64];
static RingBuf_t usart1_rx;
static uint8_t usart1_rx_byte;

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

    /* Interrupt-driven RX into the ring buffer (echo demo reads it back) */
    RingBuf_Init(&usart1_rx, usart1_rx_buf, sizeof(usart1_rx_buf));
    (void)HAL_UART_Receive_IT(&huart1, &usart1_rx_byte, 1U);

    usart1_ready = 1U;
}

/**
 * @brief HAL RX complete callback (re-arms the interrupt and stores the byte).
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1)
    {
        (void)RingBuf_Put(&usart1_rx, usart1_rx_byte);
        (void)HAL_UART_Receive_IT(&huart1, &usart1_rx_byte, 1U);
    }
}

uint16_t USART1_RxCount(void)
{
    return RingBuf_Count(&usart1_rx);
}

uint8_t USART1_RxGet(uint8_t* byte)
{
    return RingBuf_Get(&usart1_rx, byte);
}

uint16_t USART1_RxRead(uint8_t* buf, uint16_t len)
{
    return RingBuf_GetBlock(&usart1_rx, buf, len);
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
