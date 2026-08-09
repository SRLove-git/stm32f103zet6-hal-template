/**
  ******************************************************************************
  * @file    stm32f1xx_it.h
  * @brief   Interrupt service routines prototypes.
  ******************************************************************************
  */

#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* Cortex-M3 core handlers */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* Peripheral handlers used by this template */
void USART1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_IT_H */
