/**
 ******************************************************************************
 * @file    crc.h
 * @brief   Common CRC implementations (bit-by-bit, table-less).
 *
 *          - CRC-8  (poly 0x07, init 0x00, no reflection)  CRC-8/SMBUS
 *          - CRC-16 (poly 0x8005, init 0xFFFF, reflected)  CRC-16/MODBUS
 *          - CRC-32 (poly 0x04C11DB7, init 0xFFFFFFFF, reflected) IEEE 802.3
 ******************************************************************************
 */

#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    uint8_t CRC8_Compute(const uint8_t* data, uint16_t len);
    uint16_t CRC16_Modbus(const uint8_t* data, uint16_t len);
    uint32_t CRC32_Compute(const uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __CRC_H */
