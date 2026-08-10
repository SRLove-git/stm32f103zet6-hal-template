#include <stdio.h>

#include "crc.h"

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
    const uint8_t test[] = "123456789";

    CHECK(CRC8_Compute(test, 9) == 0xF4U, "CRC-8 check value 0xF4");
    CHECK(CRC16_Modbus(test, 9) == 0x4B37U, "CRC-16/Modbus check value 0x4B37");
    CHECK(CRC32_Compute(test, 9) == 0xCBF43926U, "CRC-32 check value 0xCBF43926");

    /* Empty input sanity */
    CHECK(CRC16_Modbus(test, 0) == 0xFFFFU, "CRC-16 empty = init value");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
