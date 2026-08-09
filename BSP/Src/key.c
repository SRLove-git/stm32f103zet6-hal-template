/**
 ******************************************************************************
 * @file    key.c
 * @brief   On-board keys driver.
 ******************************************************************************
 */

#include "key.h"

uint8_t KEY_Scan(void)
{
    static uint8_t last_key = KEY_NONE;
    uint8_t cur_key = KEY_NONE;

    if (HAL_GPIO_ReadPin(KEY0_PORT, KEY0_PIN) == GPIO_PIN_RESET)
    {
        cur_key = KEY0_PRESS;
    }
    else if (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET)
    {
        cur_key = KEY1_PRESS;
    }
    else if (HAL_GPIO_ReadPin(KEY_UP_PORT, KEY_UP_PIN) == GPIO_PIN_SET)
    {
        cur_key = KEY_UP_PRESS;
    }

    /* One-shot trigger: report only on the first scan while pressed */
    if ((cur_key != KEY_NONE) && (last_key == KEY_NONE))
    {
        HAL_Delay(10); /* debounce */
        last_key = cur_key;
        return cur_key;
    }

    if (cur_key == KEY_NONE)
    {
        last_key = KEY_NONE;
    }

    return KEY_NONE;
}
