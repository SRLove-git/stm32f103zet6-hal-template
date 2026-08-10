#include <math.h>
#include <stdio.h>

#include "filter.h"

#define CHECK(cond, msg)                                       \
    do                                                         \
    {                                                          \
        if (cond)                                              \
        {                                                      \
            printf("PASS: %s\n", msg);                         \
        }                                                      \
        else                                                   \
        {                                                      \
            printf("FAIL: %s\n", msg);                         \
            fails++;                                           \
        }                                                      \
    } while (0)

int main(void)
{
    int fails = 0;
    int i;
    float v;
    FILTER_MovingAvg_t ma;
    FILTER_LowPass_t lp;
    FILTER_Median_t med;
    FILTER_Kalman1D_t kal;

    /* Moving average: constant input */
    FILTER_MovingAvg_Init(&ma);
    for (i = 0; i < 20; i++)
    {
        v = FILTER_MovingAvg_Update(&ma, 10.0f);
    }
    CHECK(fabsf(v - 10.0f) < 1e-4f, "moving avg constant = 10");

    FILTER_MovingAvg_Init(&ma);
    for (i = 0; i < 20; i++)
    {
        v = FILTER_MovingAvg_Update(&ma, 20.0f);
    }
    CHECK(fabsf(v - 20.0f) < 1e-4f, "moving avg step settles at 20");

    /* Low pass: step 0 -> 100 with alpha 0.2 */
    FILTER_LowPass_Init(&lp, 0.2f);
    for (i = 0; i < 5; i++)
    {
        (void)FILTER_LowPass_Update(&lp, 0.0f);
    }
    for (i = 0; i < 100; i++)
    {
        v = FILTER_LowPass_Update(&lp, 100.0f);
    }
    CHECK(fabsf(v - 100.0f) < 1.0f, "low-pass converges to step");

    /* Median: spike removal */
    FILTER_Median_Init(&med);
    for (i = 0; i < 20; i++)
    {
        v = FILTER_Median_Update(&med, (i % 4 == 2) ? 100.0f : 0.0f);
    }
    CHECK(fabsf(v) < 1e-4f, "median rejects periodic 100 spikes");

    /* 1D Kalman: noisy measurement of 50 */
    FILTER_Kalman1D_Init(&kal, 0.01f, 100.0f);
    for (i = 0; i < 200; i++)
    {
        float noise = (float)((i * 2654435761U) % 21) - 10.0f; /* pseudo noise */
        v = FILTER_Kalman1D_Update(&kal, 50.0f + noise);
    }
    CHECK(fabsf(v - 50.0f) < 2.0f, "kalman converges to noisy 50");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
