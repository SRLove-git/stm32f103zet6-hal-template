/**
 ******************************************************************************
 * @file    led.c
 * @brief   On-board LEDs driver.
 ******************************************************************************
 */

#include "led.h"

void LED0_On(void)
{
    HAL_GPIO_WritePin(LED0_PORT, LED0_PIN, GPIO_PIN_RESET); /* active low */
}

void LED0_Off(void)
{
    HAL_GPIO_WritePin(LED0_PORT, LED0_PIN, GPIO_PIN_SET);
}

void LED0_Toggle(void)
{
    HAL_GPIO_TogglePin(LED0_PORT, LED0_PIN);
}

void LED1_On(void)
{
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET); /* active low */
}

void LED1_Off(void)
{
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);
}

void LED1_Toggle(void)
{
    HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN);
}
