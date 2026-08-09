/**
 ******************************************************************************
 * @file    beep.h
 * @brief   On-board buzzer driver (ALIENTEK Elite STM32F103ZET6).
 *
 *          BEEP - PB8 | active high (active buzzer)
 ******************************************************************************
 */

#ifndef __BEEP_H
#define __BEEP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define BEEP_PORT GPIOB
#define BEEP_PIN GPIO_PIN_8

    /* Note: GPIO pin is initialized by MX_GPIO_Init() in main.c */

    void BEEP_On(void);
    void BEEP_Off(void);
    void BEEP_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BEEP_H */
