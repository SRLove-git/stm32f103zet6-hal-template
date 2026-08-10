#include <stdio.h>
#include <string.h>

#include "ringbuf.h"

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
    uint8_t mem[16];
    uint8_t data[16];
    uint16_t n;
    uint8_t b;
    int i;
    RingBuf_t rb;

    RingBuf_Init(&rb, mem, sizeof(mem));
    CHECK(RingBuf_IsEmpty(&rb) == 1, "initially empty");
    CHECK(RingBuf_Free(&rb) == 15, "free space = size-1");

    /* Fill to capacity, then overflow */
    for (i = 0; i < 15; i++)
    {
        CHECK(RingBuf_Put(&rb, (uint8_t)i) == 1, "put while not full");
    }
    CHECK(RingBuf_IsFull(&rb) == 1, "full at size-1");
    CHECK(RingBuf_Put(&rb, 0xAA) == 0, "put fails when full");

    /* Read back in order (wrap-around test) */
    for (i = 0; i < 15; i++)
    {
        CHECK((RingBuf_Get(&rb, &b) == 1) && (b == (uint8_t)i), "get returns FIFO order");
    }
    CHECK(RingBuf_Get(&rb, &b) == 0, "get fails when empty");

    /* Block write/read */
    RingBuf_Init(&rb, mem, sizeof(mem));
    memset(data, 0x11, sizeof(data));
    n = RingBuf_PutBlock(&rb, data, 8);
    CHECK(n == 8, "block put 8 bytes");
    n = RingBuf_GetBlock(&rb, data, 16);
    CHECK(n == 8, "block get 8 bytes");
    CHECK(data[0] == 0x11 && data[7] == 0x11, "block data intact");

    /* Wrap around the tail end */
    RingBuf_Init(&rb, mem, sizeof(mem));
    for (i = 0; i < 12; i++)
    {
        (void)RingBuf_Put(&rb, (uint8_t)i);
    }
    for (i = 0; i < 6; i++)
    {
        (void)RingBuf_Get(&rb, &b);
    }
    n = RingBuf_PutBlock(&rb, data, 6);
    CHECK(n == 6, "put after partial drain wraps");
    n = RingBuf_GetBlock(&rb, data, 16);
    CHECK(n == 12, "remaining 12 bytes readable");

    printf(fails == 0 ? "ALL PASS\n" : "SOME FAILURES\n");
    return fails;
}
