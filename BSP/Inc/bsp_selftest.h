/**
 ******************************************************************************
 * @file    bsp_selftest.h
 * @brief   On-board peripheral self-test (results over USART1).
 ******************************************************************************
 */

#ifndef __BSP_SELFTEST_H
#define __BSP_SELFTEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /**
     * @brief Run the on-board peripheral self-test and print results.
     * @note  Call after USART1 is initialized (printf must be available).
     */
    void BSP_SelfTest(void);

    /** @brief 1 if BSP_SelfTest() has completed (results reusable). */
    uint8_t BSP_SelfTestRan(void);

    /** @brief 1 if the MPU6050 was detected during the self-test. */
    uint8_t BSP_MPUPresent(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SELFTEST_H */
