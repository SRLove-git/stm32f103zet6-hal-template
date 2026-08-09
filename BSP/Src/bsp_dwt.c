/**
 ******************************************************************************
 * @file    bsp_dwt.c
 * @brief   DWT-based microsecond delay implementation.
 ******************************************************************************
 */

#include "bsp_dwt.h"

static uint8_t dwt_ready = 0U;

void BSP_DWT_DelayInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    dwt_ready = 1U;
}

void BSP_DWT_DelayUs(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    if (dwt_ready == 0U)
    {
        BSP_DWT_DelayInit();
    }

    /* SystemCoreClock is 72 MHz in this template: 72 ticks per microsecond */
    ticks = us * (SystemCoreClock / 1000000U);
    start = DWT->CYCCNT;

    /* Unsigned wrap-around is intentional: works for any start value */
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}
