/**
 ******************************************************************************
 * @file    fault.h
 * @brief   HardFault crash context recorder.
 *
 *          On entry to HardFault, the exception frame (r0-r3, r12, lr, pc,
 *          xpsr) plus the Cortex-M3 fault status registers are captured in
 *          g_fault and a short report is sent over USART1.
 ******************************************************************************
 */

#ifndef __FAULT_H
#define __FAULT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

    typedef struct
    {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r12;
        uint32_t lr;
        uint32_t pc;
        uint32_t xpsr;
        uint32_t psp;   /* process stack pointer at the time of the fault */
        uint32_t cfsr;  /* configurable fault status register */
        uint32_t hfsr;  /* hard fault status register */
        uint32_t bfar;  /* bus fault address register */
        uint32_t mmfar; /* memory management fault address register */
    } FaultInfo_t;

    extern volatile FaultInfo_t g_fault;

    /**
     * @brief HardFault entry (called from the naked trampoline in it.c).
     * @param stack  pointer to the stacked exception frame.
     * @note  Does not return.
     */
    void FAULT_Handler(uint32_t* stack);

#ifdef __cplusplus
}
#endif

#endif /* __FAULT_H */
