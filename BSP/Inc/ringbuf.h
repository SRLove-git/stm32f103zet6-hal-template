/**
 ******************************************************************************
 * @file    ringbuf.h
 * @brief   Lock-free single-producer / single-consumer ring buffer.
 *
 *          Designed for ISR-to-task data flow: the ISR writes (Put), the
 *          application reads (Get). Do not call both Put and Get from the
 *          same context.
 *
 * @note    Buffer size must be a power of two; usable capacity is size - 1.
 ******************************************************************************
 */

#ifndef __RINGBUF_H
#define __RINGBUF_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    typedef struct
    {
        uint8_t* buf;
        uint16_t head; /* next write index (producer) */
        uint16_t tail; /* next read index  (consumer) */
        uint16_t size;
    } RingBuf_t;

    void RingBuf_Init(RingBuf_t* rb, uint8_t* buf, uint16_t size);

    uint16_t RingBuf_Count(const RingBuf_t* rb);
    uint16_t RingBuf_Free(const RingBuf_t* rb);
    uint8_t RingBuf_IsEmpty(const RingBuf_t* rb);
    uint8_t RingBuf_IsFull(const RingBuf_t* rb);

    uint8_t RingBuf_Put(RingBuf_t* rb, uint8_t byte); /* 1 ok, 0 full */
    uint16_t RingBuf_PutBlock(RingBuf_t* rb, const uint8_t* data, uint16_t len);
    uint8_t RingBuf_Get(RingBuf_t* rb, uint8_t* byte); /* 1 ok, 0 empty */
    uint16_t RingBuf_GetBlock(RingBuf_t* rb, uint8_t* data, uint16_t max);

#ifdef __cplusplus
}
#endif

#endif /* __RINGBUF_H */
