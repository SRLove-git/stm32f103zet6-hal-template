/**
  ******************************************************************************
  * @file    main.h
  * @brief   Common header for the STM32F103ZET6 HAL template.
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* System clock configuration: 8 MHz HSE x9 PLL = 72 MHz SYSCLK */
void SystemClock_Config(void);

/* Central error handler (blinking LEDs / infinite loop). */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
