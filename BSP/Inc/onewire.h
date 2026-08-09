/**
  ******************************************************************************
  * @file    onewire.h
  * @brief   One-wire bus on PG11 + DS18B20 temperature sensor driver.
  *
  *          1WIRE_DQ - PG11 (shared with DHT11 via the same connector U4)
  *
  * @note    Uses DWT microsecond delays (BSP_DWT_DelayUs).
  ******************************************************************************
  */

#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define OW_PORT GPIOG
#define OW_PIN  GPIO_PIN_11

void    OW_Init(void);
uint8_t OW_Reset(void);                 /* returns 1 if a device is present */

/**
  * @brief Read DS18B20 temperature.
  * @retval Temperature in 0.01 degC, e.g. 2500 = 25.00 degC.
  *         Returns 0x7FFF if no device found.
  */
int16_t DS18B20_GetTemp(void);

#ifdef __cplusplus
}
#endif

#endif /* __ONEWIRE_H */
