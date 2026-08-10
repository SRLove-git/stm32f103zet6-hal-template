/**
 ******************************************************************************
 * @file    filter.h
 * @brief   Common sensor filter algorithms (pure math, no hardware deps).
 *
 *          - FILTER_MovingAvg: sliding-window average (window 8)
 *          - FILTER_LowPass:   first-order IIR low-pass
 *          - FILTER_Median:    5-point median (removes spikes)
 *          - FILTER_Kalman1D:  single-state Kalman filter
 ******************************************************************************
 */

#ifndef __FILTER_H
#define __FILTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define FILTER_MAVG_WINDOW 8U
#define FILTER_MEDIAN_SIZE 5U

    /* Moving average */
    typedef struct
    {
        float buf[FILTER_MAVG_WINDOW];
        float sum;
        uint8_t idx;
        uint8_t count;
    } FILTER_MovingAvg_t;

    void FILTER_MovingAvg_Init(FILTER_MovingAvg_t* f);
    float FILTER_MovingAvg_Update(FILTER_MovingAvg_t* f, float x);

    /* First-order low-pass: y += alpha * (x - y), alpha in (0,1] */
    typedef struct
    {
        float alpha;
        float y;
        uint8_t init;
    } FILTER_LowPass_t;

    void FILTER_LowPass_Init(FILTER_LowPass_t* f, float alpha);
    float FILTER_LowPass_Update(FILTER_LowPass_t* f, float x);

    /* Median filter */
    typedef struct
    {
        float buf[FILTER_MEDIAN_SIZE];
        uint8_t idx;
        uint8_t count;
    } FILTER_Median_t;

    void FILTER_Median_Init(FILTER_Median_t* f);
    float FILTER_Median_Update(FILTER_Median_t* f, float x);

    /* 1D Kalman: process noise q, measurement noise r */
    typedef struct
    {
        float q;
        float r;
        float x;
        float p;
        uint8_t init;
    } FILTER_Kalman1D_t;

    void FILTER_Kalman1D_Init(FILTER_Kalman1D_t* f, float q, float r);
    float FILTER_Kalman1D_Update(FILTER_Kalman1D_t* f, float z);

#ifdef __cplusplus
}
#endif

#endif /* __FILTER_H */
