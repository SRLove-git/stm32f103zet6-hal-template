/**
  ******************************************************************************
  * @file    lsens.c
  * @brief   Light sensor driver (polling single conversion).
  ******************************************************************************
  */

#include "lsens.h"

ADC_HandleTypeDef hadc3;

void LSENS_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc3.Instance                   = LSENS_ADC;
    hadc3.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc3.Init.ContinuousConvMode    = DISABLE;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc3.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc3.Init.NbrOfConversion       = 1;

    if (HAL_ADC_Init(&hadc3) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel      = LSENS_ADC_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

uint32_t LSENS_ReadADC(void)
{
    if (HAL_ADC_Start(&hadc3) != HAL_OK)
    {
        return 0U;
    }
    if (HAL_ADC_PollForConversion(&hadc3, 10U) != HAL_OK)
    {
        return 0U;
    }
    return (uint32_t)HAL_ADC_GetValue(&hadc3);
}

uint32_t LSENS_ReadMv(void)
{
    return (LSENS_ReadADC() * 3300U) / 4095U;
}
