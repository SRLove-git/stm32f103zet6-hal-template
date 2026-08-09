/**
 ******************************************************************************
 * @file    attitude.h
 * @brief   Quaternion attitude filters (Mahony / Madgwick, IMU 6-axis).
 *
 *          Pure math module - no hardware dependency. Feed it gyro (rad/s)
 *          and accelerometer (any units, normalized internally) samples.
 *
 *          Based on S.O.H. Madgwick's reference implementation
 *          (https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/), MIT.
 ******************************************************************************
 */

#ifndef __ATTITUDE_H
#define __ATTITUDE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ATT_DEG2RAD 0.01745329252f
#define ATT_RAD2DEG 57.295779513f

    typedef enum
    {
        ATT_FILTER_MAHONY,  /* complementary: gyro + accel cross-product error */
        ATT_FILTER_MADGWICK /* gradient-descent quaternion correction          */
    } ATT_Filter_t;

    /**
     * @brief Select the active filter algorithm (default: Mahony).
     */
    void ATT_SetFilter(ATT_Filter_t filter);

    /**
     * @brief Reset the filter to identity quaternion.
     */
    void ATT_Init(void);

    /**
     * @brief One filter update (IMU: gyro + accelerometer).
     * @param gx/gy/gz  gyro in rad/s
     * @param ax/ay/az  accelerometer in g (normalized internally)
     * @param dt        elapsed time since the previous update, seconds
     */
    void ATT_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt);

    /**
     * @brief Get the quaternion [q0, q1, q2, q3].
     */
    void ATT_GetQuat(float q[4]);

    /**
     * @brief Get Euler angles in degrees: roll (X), pitch (Y), yaw (Z).
     * @note  Yaw has no absolute reference without a magnetometer (drifts).
     */
    void ATT_GetEuler(float* roll, float* pitch, float* yaw);

#ifdef __cplusplus
}
#endif

#endif /* __ATTITUDE_H */
