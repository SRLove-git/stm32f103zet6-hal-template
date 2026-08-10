#include <math.h>
#include <stdio.h>

#include "attitude.h"

#define CHECK(cond, msg)                                       \
    do                                                         \
    {                                                          \
        if (cond)                                              \
        {                                                      \
            printf("  PASS: %s\n", msg);                       \
        }                                                      \
        else                                                   \
        {                                                      \
            printf("  FAIL: %s\n", msg);                       \
            fails++;                                           \
        }                                                      \
    } while (0)

static int run_filter(ATT_Filter_t filter, const char* name)
{
    int i;
    int fails = 0;
    float r, p, y;

    ATT_SetFilter(filter);
    ATT_Init();

    /* 1. Static on a level surface: converge to roll/pitch ~ 0 */
    for (i = 0; i < 1000; i++)
    {
        ATT_UpdateIMU(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.01f);
    }
    ATT_GetEuler(&r, &p, &y);
    printf("%s static : roll=%6.2f pitch=%6.2f yaw=%6.2f\n", name, r, p, y);
    CHECK((fabsf(r) < 3.0f) && (fabsf(p) < 3.0f), "static converges to level");

    /* 2. Rotate around the sensor Y axis at 0.5 rad/s for 2 s */
    for (i = 0; i < 200; i++)
    {
        ATT_UpdateIMU(0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.01f);
    }
    ATT_GetEuler(&r, &p, &y);
    printf("%s rotated: roll=%6.2f pitch=%6.2f yaw=%6.2f\n", name, r, p, y);
    CHECK(fabsf(p) > 10.0f, "gyro integration moves pitch");

    /* 3. Hold still: accelerometer pulls attitude back to level */
    for (i = 0; i < 2000; i++)
    {
        ATT_UpdateIMU(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.01f);
    }
    ATT_GetEuler(&r, &p, &y);
    printf("%s settled: roll=%6.2f pitch=%6.2f yaw=%6.2f\n", name, r, p, y);
    CHECK(fabsf(p) < 5.0f, "accelerometer feedback converges back");

    return fails;
}

int main(void)
{
    int fails = 0;

    fails += run_filter(ATT_FILTER_MAHONY, "mahony  ");
    fails += run_filter(ATT_FILTER_MADGWICK, "madgwick");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
