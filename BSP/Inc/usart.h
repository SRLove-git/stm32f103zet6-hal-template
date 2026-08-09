/**
  ******************************************************************************
  * @file    usart.h
  * @brief   USART1 driver (ALIENTEK Elite STM32F103ZET6).
  *
  *          USART1 - PA9 (TX) / PA10 (RX), 115200-8N1
  *          Connects to the on-board CH340C USB-UART via the P3 jumpers.
  ******************************************************************************
  */

#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1;

/* Initialize USART1 (clocks/pins/NVIC handled by HAL_UART_MspInit in
 * stm32f1xx_hal_msp.c) and redirect printf()/puts() to it. */
void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
