/**
 ******************************************************************************
 * @file    demo.h
 * @brief   Shared demo logic: keys + LCD attitude display.
 *
 *          Used by both the bare-metal main loop and the FreeRTOS tasks
 *          (Core/Src/freertos.c), so the demo behavior stays identical.
 ******************************************************************************
 */

#ifndef __DEMO_H
#define __DEMO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

    /**
     * @brief Initialize the attitude demo (MPU check + LCD setup).
     * @retval 1 if the attitude display is active (MPU6050 present), 0 otherwise.
     */
    uint8_t Demo_Init(void);

    /**
     * @brief Scan the keys once and handle the actions.
     */
    void Demo_KeyScan(void);

    /**
     * @brief Refresh the LCD attitude display once (or LED heartbeat if no MPU).
     */
    void Demo_AttitudeUpdate(void);

    /**
     * @brief Pump pending UART RX bytes into the CLI parser.
     */
    void Demo_CliPoll(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEMO_H */
