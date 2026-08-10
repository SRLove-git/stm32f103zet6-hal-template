/**
 ******************************************************************************
 * @file    pid.h
 * @brief   Positional PID controller (pure math, no hardware deps).
 *
 *          Features:
 *            - integral anti-windup (term clamped to the output range)
 *            - output clamping
 *            - derivative on the measurement (no derivative kick on setpoint
 *              changes)
 ******************************************************************************
 */

#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    typedef struct
    {
        float kp;
        float ki;
        float kd;
        float out_min;
        float out_max;
        float integral;
        float prev_measurement;
        float out;
        uint8_t init;
    } PID_t;

    /**
     * @brief Initialize the PID gains and output limits.
     */
    void PID_Init(PID_t* pid, float kp, float ki, float kd, float out_min, float out_max);

    /**
     * @brief Clear the integral and derivative state.
     */
    void PID_Reset(PID_t* pid);

    /**
     * @brief One controller update.
     * @param setpoint    desired value
     * @param measurement current measurement
     * @param dt          elapsed time since the previous update, seconds
     * @retval controller output, clamped to [out_min, out_max]
     */
    float PID_Update(PID_t* pid, float setpoint, float measurement, float dt);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
