/**
 ******************************************************************************
 * @file    bsp_dwt.h
 * @brief   Microsecond delay using the DWT cycle counter (Cortex-M3).
 *
 *          Needed by bit-banged peripherals (DS18B20/DHT11, NEC IR) where
 *          HAL_Delay() (millisecond resolution) is too coarse.
 ******************************************************************************
 */

#ifndef __BSP_DWT_H
#define __BSP_DWT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

    /**
     * @brief Initialize the DWT cycle counter.
     * @note  Called automatically by BSP_DWT_DelayUs() on first use.
     */
    void BSP_DWT_DelayInit(void);

    /**
     * @brief Busy-wait for the given number of microseconds.
     * @param us  Delay in microseconds (blocking).
     */
    void BSP_DWT_DelayUs(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DWT_H */
