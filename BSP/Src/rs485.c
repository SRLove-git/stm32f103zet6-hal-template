/**
 ******************************************************************************
 * @file    rs485.c
 * @brief   RS485 driver (half-duplex polling).
 ******************************************************************************
 */

#include "rs485.h"

UART_HandleTypeDef huart2;

static void RS485_SetDirTransmit(void)
{
    HAL_GPIO_WritePin(RS485_RE_PORT, RS485_RE_PIN, GPIO_PIN_SET);
}

static void RS485_SetDirReceive(void)
{
    HAL_GPIO_WritePin(RS485_RE_PORT, RS485_RE_PIN, GPIO_PIN_RESET);
}

void RS485_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = RS485_RE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RS485_RE_PORT, &GPIO_InitStruct);
    RS485_SetDirReceive();

    huart2.Instance = RS485_UART;
    huart2.Init.BaudRate = 9600; /* match the remote device; 115200 also works */
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

HAL_StatusTypeDef RS485_SendData(uint8_t* buf, uint16_t len)
{
    HAL_StatusTypeDef ret;

    RS485_SetDirTransmit();
    ret = HAL_UART_Transmit(&huart2, buf, len, RS485_TIMEOUT);
    RS485_SetDirReceive();
    return ret;
}

HAL_StatusTypeDef RS485_ReceiveData(uint8_t* buf, uint16_t len)
{
    return HAL_UART_Receive(&huart2, buf, len, RS485_TIMEOUT);
}
