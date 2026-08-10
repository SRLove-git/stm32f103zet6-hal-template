/**
 ******************************************************************************
 * @file    ringbuf.c
 * @brief   Ring buffer implementation (power-of-two size, mask wraparound).
 ******************************************************************************
 */

#include "ringbuf.h"

void RingBuf_Init(RingBuf_t* rb, uint8_t* buf, uint16_t size)
{
    rb->buf = buf;
    rb->head = 0U;
    rb->tail = 0U;
    rb->size = size;
}

uint16_t RingBuf_Count(const RingBuf_t* rb)
{
    return (uint16_t)((rb->head - rb->tail) & (rb->size - 1U));
}

uint16_t RingBuf_Free(const RingBuf_t* rb)
{
    return (uint16_t)(rb->size - 1U - RingBuf_Count(rb));
}

uint8_t RingBuf_IsEmpty(const RingBuf_t* rb)
{
    return (rb->head == rb->tail) ? 1U : 0U;
}

uint8_t RingBuf_IsFull(const RingBuf_t* rb)
{
    return (RingBuf_Count(rb) == (uint16_t)(rb->size - 1U)) ? 1U : 0U;
}

uint8_t RingBuf_Put(RingBuf_t* rb, uint8_t byte)
{
    uint16_t next = (uint16_t)((rb->head + 1U) & (rb->size - 1U));

    if (next == rb->tail)
    {
        return 0U; /* full */
    }

    rb->buf[rb->head] = byte;
    rb->head = next;
    return 1U;
}

uint16_t RingBuf_PutBlock(RingBuf_t* rb, const uint8_t* data, uint16_t len)
{
    uint16_t written = 0U;

    while ((written < len) && (RingBuf_Put(rb, data[written]) != 0U))
    {
        written++;
    }
    return written;
}

uint8_t RingBuf_Get(RingBuf_t* rb, uint8_t* byte)
{
    if (rb->head == rb->tail)
    {
        return 0U; /* empty */
    }

    *byte = rb->buf[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1U) & (rb->size - 1U));
    return 1U;
}

uint16_t RingBuf_GetBlock(RingBuf_t* rb, uint8_t* data, uint16_t max)
{
    uint16_t read = 0U;

    while ((read < max) && (RingBuf_Get(rb, &data[read]) != 0U))
    {
        read++;
    }
    return read;
}
