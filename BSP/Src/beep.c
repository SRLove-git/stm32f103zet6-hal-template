/**
  ******************************************************************************
  * @file    beep.c
  * @brief   On-board buzzer driver.
  ******************************************************************************
  */

#include "beep.h"

void BEEP_On(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET); /* active high */
}

void BEEP_Off(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
}

void BEEP_Toggle(void)
{
    HAL_GPIO_TogglePin(BEEP_PORT, BEEP_PIN);
}
