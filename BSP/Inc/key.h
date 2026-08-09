/**
 ******************************************************************************
 * @file    key.h
 * @brief   On-board keys driver (ALIENTEK Elite STM32F103ZET6).
 *
 *          KEY0   - PE4 | active low  (internal pull-up)
 *          KEY1   - PE3 | active low  (internal pull-up)
 *          KEY_UP - PA0 | active high (internal pull-down, WKUP wake-up)
 ******************************************************************************
 */

#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define KEY0_PORT GPIOE
#define KEY0_PIN GPIO_PIN_4

#define KEY1_PORT GPIOE
#define KEY1_PIN GPIO_PIN_3

#define KEY_UP_PORT GPIOA
#define KEY_UP_PIN GPIO_PIN_0

/* Return codes of KEY_Scan() */
#define KEY_NONE 0U
#define KEY0_PRESS 1U
#define KEY1_PRESS 2U
#define KEY_UP_PRESS 3U

    /* Note: GPIO pins are initialized by MX_GPIO_Init() in main.c */

    /**
     * @brief Scans all keys with software debounce.
     * @retval KEY_NONE / KEY0_PRESS / KEY1_PRESS / KEY_UP_PRESS
     *         (one-shot: a key is reported once per press)
     */
    uint8_t KEY_Scan(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */
