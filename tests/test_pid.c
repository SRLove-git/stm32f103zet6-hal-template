#include <math.h>
#include <stdio.h>

#include "pid.h"

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

/* First-order plant: dy/dt = K*u - y  (steady state y = K*u) */
static float plant(float y, float u, float k, float dt)
{
    return y + (k * u - y) * dt;
}

int main(void)
{
    int fails = 0;
    int i;
    float y;
    float u;
    PID_t pid;

    /* PI on a first-order plant: must reach the setpoint (no offset) */
    PID_Init(&pid, 1.0f, 2.0f, 0.0f, -200.0f, 200.0f);
    y = 0.0f;
    for (i = 0; i < 2000; i++)
    {
        u = PID_Update(&pid, 100.0f, y, 0.01f);
        y = plant(y, u, 2.0f, 0.01f);
    }
    printf("PI: y=%.2f u=%.2f\n", y, u);
    CHECK(fabsf(y - 100.0f) < 0.5f, "PI converges to setpoint without offset");
    CHECK(fabsf(u) < 201.0f, "PI output stays in range");

    /* P-only: proportional offset remains, output bounded */
    PID_Init(&pid, 1.0f, 0.0f, 0.0f, -200.0f, 200.0f);
    y = 0.0f;
    for (i = 0; i < 2000; i++)
    {
        u = PID_Update(&pid, 100.0f, y, 0.01f);
        y = plant(y, u, 2.0f, 0.01f);
    }
    printf("P : y=%.2f u=%.2f\n", y, u);
    CHECK((y > 50.0f) && (y < 99.0f), "P-only leaves a bounded offset");

    /* Output clamping: far setpoint saturates the output */
    PID_Init(&pid, 1.0f, 2.0f, 0.0f, -10.0f, 10.0f);
    u = PID_Update(&pid, 1000.0f, 0.0f, 0.01f);
    CHECK(u == 10.0f, "output clamps at out_max");

    /* Derivative on measurement: no kick when setpoint steps */
    PID_Init(&pid, 1.0f, 0.0f, 5.0f, -200.0f, 200.0f);
    y = 0.0f;
    u = PID_Update(&pid, 0.0f, y, 0.01f);
    u = PID_Update(&pid, 100.0f, y, 0.01f);
    CHECK(fabsf(u - 100.0f) < 1e-3f, "no derivative kick on setpoint step");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
