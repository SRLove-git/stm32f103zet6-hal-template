/**
 ******************************************************************************
 * @file    ir_nec.h
 * @brief   NEC protocol IR receiver driver (polling with DWT timing).
 *
 *          REMOTE_IN - PB9 (LF0038 IR receiver, active low output)
 ******************************************************************************
 */

#ifndef __IR_NEC_H
#define __IR_NEC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define IR_PORT GPIOB
#define IR_PIN GPIO_PIN_9

    void IR_Init(void);

    /**
     * @brief Poll for a complete NEC frame.
     * @retval 32-bit NEC code (addr<<24 | ~addr<<16 | cmd<<8 | ~cmd), or 0.
     *         Should be called periodically (e.g. every 10-20 ms).
     */
    uint32_t IR_GetKey(void);

#ifdef __cplusplus
}
#endif

#endif /* __IR_NEC_H */
