/**
 ******************************************************************************
 * @file    pid.c
 * @brief   Positional PID implementation.
 ******************************************************************************
 */

#include "pid.h"

void PID_Init(PID_t* pid, float kp, float ki, float kd, float out_min, float out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integral = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->out = 0.0f;
    pid->init = 0U;
}

void PID_Reset(PID_t* pid)
{
    pid->integral = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->out = 0.0f;
    pid->init = 0U;
}

float PID_Update(PID_t* pid, float setpoint, float measurement, float dt)
{
    float err;
    float p;
    float i;
    float d;
    float out;

    if (dt <= 0.0f)
    {
        dt = 1e-3f;
    }

    err = setpoint - measurement;

    /* Proportional */
    p = pid->kp * err;

    /* Integral with anti-windup: clamp the accumulated term to the output
     * range so a long saturation does not wind the controller up. */
    pid->integral += err * dt;
    if (pid->ki != 0.0f)
    {
        float imax = pid->out_max / pid->ki;
        if (pid->integral > imax)
        {
            pid->integral = imax;
        }
        else if (pid->integral < -imax)
        {
            pid->integral = -imax;
        }
    }
    else
    {
        pid->integral = 0.0f;
    }
    i = pid->ki * pid->integral;

    /* Derivative on the measurement (no kick on setpoint changes) */
    if (pid->init != 0U)
    {
        d = pid->kd * (measurement - pid->prev_measurement) / dt;
    }
    else
    {
        d = 0.0f;
        pid->init = 1U;
    }
    pid->prev_measurement = measurement;

    out = p + i - d;
    if (out > pid->out_max)
    {
        out = pid->out_max;
    }
    else if (out < pid->out_min)
    {
        out = pid->out_min;
    }
    pid->out = out;

    return out;
}
