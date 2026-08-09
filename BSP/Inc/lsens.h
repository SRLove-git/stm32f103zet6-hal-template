/**
  ******************************************************************************
  * @file    lsens.h
  * @brief   On-board light sensor driver (ADC3 channel 6).
  *
  *          LIGHT_SENSOR - PF8 (ADC3_IN6)
  *          Brighter environment -> higher ADC value.
  ******************************************************************************
  */

#ifndef __LSENS_H
#define __LSENS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define LSENS_ADC         ADC3
#define LSENS_ADC_CHANNEL ADC_CHANNEL_6

extern ADC_HandleTypeDef hadc3;

void     LSENS_Init(void);
uint32_t LSENS_ReadADC(void);   /* raw 12-bit value 0..4095 */
uint32_t LSENS_ReadMv(void);    /* voltage 0..3300 mV       */

#ifdef __cplusplus
}
#endif

#endif /* __LSENS_H */
