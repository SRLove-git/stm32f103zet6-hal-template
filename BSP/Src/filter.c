/**
 ******************************************************************************
 * @file    filter.c
 * @brief   Sensor filter implementations.
 ******************************************************************************
 */

#include "filter.h"

void FILTER_MovingAvg_Init(FILTER_MovingAvg_t* f)
{
    uint8_t i;

    for (i = 0U; i < FILTER_MAVG_WINDOW; i++)
    {
        f->buf[i] = 0.0f;
    }
    f->sum = 0.0f;
    f->idx = 0U;
    f->count = 0U;
}

float FILTER_MovingAvg_Update(FILTER_MovingAvg_t* f, float x)
{
    if (f->count < FILTER_MAVG_WINDOW)
    {
        f->sum += x;
        f->buf[f->idx] = x;
        f->idx = (uint8_t)((f->idx + 1U) % FILTER_MAVG_WINDOW);
        f->count++;
        return f->sum / (float)f->count;
    }

    f->sum -= f->buf[f->idx];
    f->sum += x;
    f->buf[f->idx] = x;
    f->idx = (uint8_t)((f->idx + 1U) % FILTER_MAVG_WINDOW);
    return f->sum / (float)FILTER_MAVG_WINDOW;
}

void FILTER_LowPass_Init(FILTER_LowPass_t* f, float alpha)
{
    f->alpha = alpha;
    f->y = 0.0f;
    f->init = 0U;
}

float FILTER_LowPass_Update(FILTER_LowPass_t* f, float x)
{
    if (f->init == 0U)
    {
        f->y = x;
        f->init = 1U;
        return f->y;
    }

    f->y += f->alpha * (x - f->y);
    return f->y;
}

void FILTER_Median_Init(FILTER_Median_t* f)
{
    uint8_t i;

    for (i = 0U; i < FILTER_MEDIAN_SIZE; i++)
    {
        f->buf[i] = 0.0f;
    }
    f->idx = 0U;
    f->count = 0U;
}

float FILTER_Median_Update(FILTER_Median_t* f, float x)
{
    float tmp[FILTER_MEDIAN_SIZE];
    uint8_t i;
    uint8_t j;
    uint8_t n;

    f->buf[f->idx] = x;
    f->idx = (uint8_t)((f->idx + 1U) % FILTER_MEDIAN_SIZE);
    if (f->count < FILTER_MEDIAN_SIZE)
    {
        f->count++;
    }

    n = f->count;
    for (i = 0U; i < n; i++)
    {
        tmp[i] = f->buf[i];
    }

    /* Insertion sort */
    for (i = 1U; i < n; i++)
    {
        float key = tmp[i];
        j = i;
        while ((j > 0U) && (tmp[j - 1U] > key))
        {
            tmp[j] = tmp[j - 1U];
            j--;
        }
        tmp[j] = key;
    }

    return tmp[n / 2U];
}

void FILTER_Kalman1D_Init(FILTER_Kalman1D_t* f, float q, float r)
{
    f->q = q;
    f->r = r;
    f->x = 0.0f;
    f->p = 1.0f;
    f->init = 0U;
}

float FILTER_Kalman1D_Update(FILTER_Kalman1D_t* f, float z)
{
    float k;

    if (f->init == 0U)
    {
        f->x = z;
        f->p = 1.0f;
        f->init = 1U;
        return f->x;
    }

    /* Prediction */
    f->p += f->q;
    /* Update */
    k = f->p / (f->p + f->r);
    f->x += k * (z - f->x);
    f->p *= (1.0f - k);

    return f->x;
}
