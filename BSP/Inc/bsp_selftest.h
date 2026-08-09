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

    /**
     * @brief Run the on-board peripheral self-test and print results.
     * @note  Call after USART1 is initialized (printf must be available).
     */
    void BSP_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SELFTEST_H */
