/**
 ******************************************************************************
 * @file    crc.c
 * @brief   CRC-8 / CRC-16-Modbus / CRC-32 implementations.
 ******************************************************************************
 */

#include "crc.h"

uint8_t CRC8_Compute(const uint8_t* data, uint16_t len)
{
    uint8_t crc = 0U;
    uint16_t i;
    uint8_t bit;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            }
            else
            {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }
    return crc;
}

uint16_t CRC16_Modbus(const uint8_t* data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;
    uint8_t bit;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc = (uint16_t)(crc >> 1U);
            }
        }
    }
    return crc;
}

uint32_t CRC32_Compute(const uint8_t* data, uint16_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    uint16_t i;
    uint8_t bit;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x00000001U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xEDB88320U;
            }
            else
            {
                crc = crc >> 1U;
            }
        }
    }
    return crc ^ 0xFFFFFFFFU;
}
