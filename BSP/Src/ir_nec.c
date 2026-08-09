/**
 ******************************************************************************
 * @file    ir_nec.c
 * @brief   NEC IR protocol decoder (bit-banged).
 ******************************************************************************
 */

#include "ir_nec.h"
#include "bsp_dwt.h"

#define IR_ACTIVE GPIO_PIN_RESET /* receiver output is low when signal arrives */

/* Timing windows (us) */
#define IR_START_LOW_MIN 8000U
#define IR_START_LOW_MAX 11000U
#define IR_START_HIGH_MIN 3000U
#define IR_START_HIGH_MAX 6500U
#define IR_BIT_LOW_MAX 1400U
#define IR_BIT_HIGH_0_MAX 1200U
#define IR_BIT_HIGH_1_MIN 1500U
#define IR_BIT_HIGH_1_MAX 2200U

static uint32_t IR_WaitLevel(GPIO_PinState level, uint32_t timeout_us)
{
    uint32_t elapsed = 0U;
    while (elapsed < timeout_us)
    {
        if (HAL_GPIO_ReadPin(IR_PORT, IR_PIN) == level)
        {
            return elapsed;
        }
        BSP_DWT_DelayUs(10U);
        elapsed += 10U;
    }
    return timeout_us; /* timed out */
}

void IR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    BSP_DWT_DelayInit();

    GPIO_InitStruct.Pin = IR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; /* idle high */
    HAL_GPIO_Init(IR_PORT, &GPIO_InitStruct);
}

uint32_t IR_GetKey(void)
{
    uint32_t code = 0U;
    uint32_t t;
    uint8_t i, bit;

    /* Wait for the frame to start: idle high -> falling edge (active low) */
    if (HAL_GPIO_ReadPin(IR_PORT, IR_PIN) == IR_ACTIVE)
    {
        return 0U; /* already in the middle of a frame */
    }
    t = IR_WaitLevel(IR_ACTIVE, 300000U);
    if (t >= 300000U)
    {
        return 0U; /* no signal within 300 ms */
    }

    /* 9 ms start low pulse */
    t = IR_WaitLevel(GPIO_PIN_SET, 15000U);
    if ((t < IR_START_LOW_MIN) || (t > IR_START_LOW_MAX))
    {
        return 0U;
    }

    /* 4.5 ms high pulse */
    t = IR_WaitLevel(IR_ACTIVE, 15000U);
    if ((t < IR_START_HIGH_MIN) || (t > IR_START_HIGH_MAX))
    {
        return 0U;
    }

    /* 32 data bits, LSB first */
    for (i = 0U; i < 32U; i++)
    {
        /* low pulse ~560 us */
        t = IR_WaitLevel(GPIO_PIN_SET, IR_BIT_LOW_MAX + 800U);
        if (t > IR_BIT_LOW_MAX)
        {
            return 0U;
        }

        /* high pulse decides bit value */
        t = IR_WaitLevel(IR_ACTIVE, IR_BIT_HIGH_1_MAX + 800U);
        if (t <= IR_BIT_HIGH_0_MAX)
        {
            bit = 0U;
        }
        else if ((t >= IR_BIT_HIGH_1_MIN) && (t <= IR_BIT_HIGH_1_MAX))
        {
            bit = 1U;
        }
        else
        {
            return 0U;
        }

        code >>= 1U;
        if (bit != 0U)
        {
            code |= 0x80000000U;
        }
    }

    /* Stop bit: trailing 560 us low */
    t = IR_WaitLevel(GPIO_PIN_SET, IR_BIT_LOW_MAX + 800U);
    if (t > IR_BIT_LOW_MAX)
    {
        return 0U;
    }

    return code;
}
