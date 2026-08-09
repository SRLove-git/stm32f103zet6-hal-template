/**
  ******************************************************************************
  * @file    stm32f1xx_hal_msp.c
  * @brief   HAL MSP module (peripheral low-level initialization).
  *
  *          Follows the STM32CubeMX convention: MSP = MCU Support Package,
  *          i.e. clocks, pins and NVIC belonging to each peripheral.
  ******************************************************************************
  */

#include "main.h"
#include "sd_card.h"
#include "usart.h"

/**
  * @brief Initializes the global MSP.
  *        (AFIO/PWR clocks, interrupt priority grouping, SysTick priority)
  */
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* 4 bits preemption priority, 0 bits subpriority (Cortex-M3 default) */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * @brief USART MSP Initialization.
  *        USART1: PA9 (TX) / PA10 (RX), 115200-8N1, connected to CH340C
  *        through the P3 jumpers on the Elite board.
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* USART1 GPIO Configuration: PA9 -> TX (AF push-pull),
         *                            PA10 -> RX (floating input) */
        GPIO_InitStruct.Pin       = GPIO_PIN_9;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* SysTick keeps priority 0; USART1 uses priority 1 */
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else if (huart->Instance == USART2)
    {
        /* RS485 (P5 jumpers to TP8485): PA2 -> TX, PA3 -> RX */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_2;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin   = GPIO_PIN_3;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
  * @brief USART MSP De-Initialization.
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_DISABLE();

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if (huart->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
    }
}

/**
  * @brief I2C MSP Initialization. I2C1: PB6 (SCL) / PB7 (SDA), 24C02 EEPROM.
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;   /* open-drain, board pull-ups */
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

/**
  * @brief I2C MSP De-Initialization.
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
    }
}

/**
  * @brief SPI MSP Initialization. SPI2: PB13 (SCK) / PB14 (MISO) / PB15 (MOSI).
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hspi->Instance == SPI2)
    {
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_13 | GPIO_PIN_15; /* SCK, MOSI */
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin   = GPIO_PIN_14;               /* MISO */
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

/**
  * @brief SPI MSP De-Initialization.
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    }
}

/**
  * @brief ADC MSP Initialization. ADC3: PF8 (light sensor, IN6).
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hadc->Instance == ADC3)
    {
        __HAL_RCC_ADC3_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();

        GPIO_InitStruct.Pin  = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    }
}

/**
  * @brief ADC MSP De-Initialization.
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC3)
    {
        __HAL_RCC_ADC3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_8);
    }
}

/**
  * @brief CAN MSP Initialization. CAN1: PA11 (RX) / PA12 (TX), P6 = CAN.
  */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hcan->Instance == CAN1)
    {
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_11;               /* RX */
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin   = GPIO_PIN_12;               /* TX */
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
  * @brief CAN MSP De-Initialization.
  */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        __HAL_RCC_CAN1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
    }
}

/**
  * @brief SDIO MSP Initialization: PC8..PC11 (D0..D3), PC12 (CK), PD2 (CMD).
  */
void HAL_SD_MspInit(SD_HandleTypeDef *hsdi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hsdi->Instance == SDIO)
    {
        __HAL_RCC_SDIO_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                               GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin   = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(SDIO_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(SDIO_IRQn);
    }
}

/**
  * @brief SDIO MSP De-Initialization.
  */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsdi)
{
    if (hsdi->Instance == SDIO)
    {
        __HAL_RCC_SDIO_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                               GPIO_PIN_11 | GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
        HAL_NVIC_DisableIRQ(SDIO_IRQn);
    }
}
