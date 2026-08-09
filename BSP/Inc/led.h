/**
  ******************************************************************************
  * @file    led.h
  * @brief   On-board LEDs driver (ALIENTEK Elite STM32F103ZET6).
  *
  *          LED0 - PB5 (red)   | active low
  *          LED1 - PE5 (green) | active low
  ******************************************************************************
  */

#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define LED0_PORT GPIOB
#define LED0_PIN  GPIO_PIN_5

#define LED1_PORT GPIOE
#define LED1_PIN  GPIO_PIN_5

/* Note: GPIO pins are initialized by MX_GPIO_Init() in main.c */

void LED0_On(void);
void LED0_Off(void);
void LED0_Toggle(void);

void LED1_On(void);
void LED1_Off(void);
void LED1_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */
