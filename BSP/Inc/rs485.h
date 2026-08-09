/**
  ******************************************************************************
  * @file    rs485.h
  * @brief   On-board RS485 driver (USART2 + direction control).
  *
  *          TX  - PA2 | RX - PA3 | RE/DE - PD7 (high = transmit)
  *          The P5 jumpers must connect USART2 to the TP8485 transceiver.
  ******************************************************************************
  */

#ifndef __RS485_H
#define __RS485_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define RS485_UART     USART2
#define RS485_RE_PORT  GPIOD
#define RS485_RE_PIN   GPIO_PIN_7
#define RS485_TIMEOUT  100U

extern UART_HandleTypeDef huart2;

void RS485_Init(void);
HAL_StatusTypeDef RS485_SendData(uint8_t *buf, uint16_t len);
HAL_StatusTypeDef RS485_ReceiveData(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __RS485_H */
