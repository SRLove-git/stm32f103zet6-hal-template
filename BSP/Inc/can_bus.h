/**
 ******************************************************************************
 * @file    can_bus.h
 * @brief   On-board CAN driver (CAN1).
 *
 *          RX - PA11 | TX - PA12 (500 kbit/s, 11-bit standard ID)
 *          The P6 jumpers must select CAN (shared with USB).
 ******************************************************************************
 */

#ifndef __CAN_BUS_H
#define __CAN_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

    extern CAN_HandleTypeDef hcan1;

    void CAN1_Init(void);

    /**
     * @brief Internal loopback self-test (no external bus needed).
     *        Switches to CAN_MODE_LOOPBACK, sends a frame and checks the echo.
     * @retval HAL_OK on success.
     */
    HAL_StatusTypeDef CAN1_SelfTest(void);

    /**
     * @brief Send a standard-ID data frame (blocking polling).
     * @retval HAL_OK on success.
     */
    HAL_StatusTypeDef CAN1_SendMsg(uint32_t std_id, uint8_t* data, uint8_t len);

    /**
     * @brief Poll the RX FIFO for one frame.
     * @retval number of received bytes, or 0 if the FIFO is empty.
     */
    uint8_t CAN1_ReceiveMsg(uint32_t* std_id, uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_BUS_H */
