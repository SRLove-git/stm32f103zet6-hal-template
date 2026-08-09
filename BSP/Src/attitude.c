/**
 ******************************************************************************
 * @file    attitude.c
 * @brief   Mahony / Madgwick quaternion filters (IMU-only variants).
 *
 *          Based on S.O.H. Madgwick's open-source reference implementation
 *          (https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/), MIT.
 ******************************************************************************
 */

#include "attitude.h"

#include <math.h>

/* Filter gains (2*Kp / 2*Ki). Ki = 0 for IMU-only: without a magnetometer
 * the integral term has no heading reference to correct against. */
#define ATT_TWO_KP 1.0f
#define ATT_TWO_KI 0.0f
#define ATT_MADGWICK_BETA 0.1f /* Madgwick gradient-descent gain */

static float q0 = 1.0f;
static float q1 = 0.0f;
static float q2 = 0.0f;
static float q3 = 0.0f;

static ATT_Filter_t att_filter = ATT_FILTER_MAHONY;

static float integral_fbx = 0.0f;
static float integral_fby = 0.0f;
static float integral_fbz = 0.0f;

static float ATT_InvSqrt(float x)
{
    return 1.0f / sqrtf(x);
}

void ATT_Init(void)
{
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    integral_fbx = 0.0f;
    integral_fby = 0.0f;
    integral_fbz = 0.0f;
}

void ATT_SetFilter(ATT_Filter_t filter)
{
    att_filter = filter;
}

static void ATT_UpdateMahony(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    float recip_norm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    /* Integrate the gyro-only quaternion rate first */
    gx *= 0.5f * dt;
    gy *= 0.5f * dt;
    gz *= 0.5f * dt;
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    /* Accelerometer feedback (skip if the sample is invalid) */
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {
        recip_norm = ATT_InvSqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        /* Estimated direction of gravity in the sensor frame */
        halfvx = q1 * q3 - q0 * q2;
        halfvy = q0 * q1 + q2 * q3;
        halfvz = q0 * q0 - 0.5f + q3 * q3;

        /* Error = cross product of measured and estimated gravity */
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        if (ATT_TWO_KI > 0.0f)
        {
            integral_fbx += ATT_TWO_KI * halfex * dt;
            integral_fby += ATT_TWO_KI * halfey * dt;
            integral_fbz += ATT_TWO_KI * halfez * dt;
            gx += integral_fbx;
            gy += integral_fby;
            gz += integral_fbz;
        }
        else
        {
            integral_fbx = 0.0f;
            integral_fby = 0.0f;
            integral_fbz = 0.0f;
        }

        /* Proportional feedback */
        gx += ATT_TWO_KP * halfex;
        gy += ATT_TWO_KP * halfey;
        gz += ATT_TWO_KP * halfez;
    }

    /* Integrate the corrected rate into the quaternion */
    gx *= 0.5f * dt;
    gy *= 0.5f * dt;
    gz *= 0.5f * dt;
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    /* Normalize */
    recip_norm = ATT_InvSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recip_norm;
    q1 *= recip_norm;
    q2 *= recip_norm;
    q3 *= recip_norm;
}

static void ATT_UpdateMadgwick(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    float recip_norm;
    float qdot1, qdot2, qdot3, qdot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2;
    float q0q0, q1q1, q2q2, q3q3;
    float s0, s1, s2, s3;

    /* Rate of change of quaternion from gyroscope */
    qdot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qdot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
    qdot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
    qdot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

    /* Gradient-descent corrective step from the accelerometer */
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
    {
        recip_norm = ATT_InvSqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _4q0 = 4.0f * q0;
        _4q1 = 4.0f * q1;
        _4q2 = 4.0f * q2;
        _8q1 = 8.0f * q1;
        _8q2 = 8.0f * q2;
        q0q0 = q0 * q0;
        q1q1 = q1 * q1;
        q2q2 = q2 * q2;
        q3q3 = q3 * q3;

        /* Objective function gradient (see Madgwick's paper) */
        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 +
             _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 +
             _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;

        /* Skip if the gradient is (numerically) zero: normalizing it would
         * divide by zero and produce NaN (e.g. exact alignment with gravity). */
        if ((s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3) > 1e-10f)
        {
            recip_norm = ATT_InvSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
            s0 *= recip_norm;
            s1 *= recip_norm;
            s2 *= recip_norm;
            s3 *= recip_norm;

            /* Apply the corrective step */
            qdot1 -= ATT_MADGWICK_BETA * s0;
            qdot2 -= ATT_MADGWICK_BETA * s1;
            qdot3 -= ATT_MADGWICK_BETA * s2;
            qdot4 -= ATT_MADGWICK_BETA * s3;
        }
    }

    /* Integrate and normalize */
    q0 += qdot1 * dt;
    q1 += qdot2 * dt;
    q2 += qdot3 * dt;
    q3 += qdot4 * dt;

    recip_norm = ATT_InvSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recip_norm;
    q1 *= recip_norm;
    q2 *= recip_norm;
    q3 *= recip_norm;
}

void ATT_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    if (att_filter == ATT_FILTER_MADGWICK)
    {
        ATT_UpdateMadgwick(gx, gy, gz, ax, ay, az, dt);
    }
    else
    {
        ATT_UpdateMahony(gx, gy, gz, ax, ay, az, dt);
    }
}

void ATT_GetQuat(float q[4])
{
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
}

void ATT_GetEuler(float* roll, float* pitch, float* yaw)
{
    float sp = 2.0f * (q0 * q2 - q3 * q1);

    if (sp > 1.0f)
    {
        sp = 1.0f;
    }
    else if (sp < -1.0f)
    {
        sp = -1.0f;
    }

    *roll = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * ATT_RAD2DEG;
    *pitch = asinf(sp) * ATT_RAD2DEG;
    *yaw = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * ATT_RAD2DEG;
}
