/**
 ******************************************************************************
 * @file    oled.c
 * @brief   ATK-OLED SSD1306 driver (8080 parallel interface).
 ******************************************************************************
 */

#include "oled.h"
#include "lcdfont.h"
#include "bsp_dwt.h"

#define OLED_DC_PORT GPIOD
#define OLED_DC_PIN GPIO_PIN_3
#define OLED_CS_PORT GPIOD
#define OLED_CS_PIN GPIO_PIN_6

#define OLED_WR_PORT GPIOG
#define OLED_WR_PIN GPIO_PIN_14
#define OLED_RD_PORT GPIOG
#define OLED_RD_PIN GPIO_PIN_13
#define OLED_RST_PORT GPIOG
#define OLED_RST_PIN GPIO_PIN_15

#define OLED_DATA_PORT GPIOC

static uint8_t oled_buf[OLED_HEIGHT / 8U][OLED_WIDTH];

static void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
    HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, (cmd != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_RESET);

    /* PC0..PC7 = D0..D7; keep PC8..PC15 (SDIO) untouched */
    OLED_DATA_PORT->ODR = (OLED_DATA_PORT->ODR & 0xFF00U) | dat;

    HAL_GPIO_WritePin(OLED_WR_PORT, OLED_WR_PIN, GPIO_PIN_RESET);
    BSP_DWT_DelayUs(1U);
    HAL_GPIO_WritePin(OLED_WR_PORT, OLED_WR_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET);
}

static void OLED_SetPos(uint8_t x, uint8_t page)
{
    OLED_WR_Byte((uint8_t)(0xB0U + page), 0U);      /* page address */
    OLED_WR_Byte((uint8_t)(x & 0x0FU), 0U);         /* low column */
    OLED_WR_Byte((uint8_t)(0x10U | (x >> 4U)), 0U); /* high column */
}

void OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    BSP_DWT_DelayInit();

    /* D0..D7 on PC0..PC7 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                          GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_DATA_PORT, &GPIO_InitStruct);

    /* Control pins: DC/CS on PD, WR/RD/RST on PG */
    GPIO_InitStruct.Pin = OLED_DC_PIN | OLED_CS_PIN;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OLED_WR_PIN | OLED_RD_PIN | OLED_RST_PIN;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    HAL_GPIO_WritePin(OLED_RD_PORT, OLED_RD_PIN, GPIO_PIN_SET); /* no reads */
    HAL_GPIO_WritePin(OLED_WR_PORT, OLED_WR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET);

    /* Hardware reset */
    HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10U);
    HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(10U);

    /* SSD1306 init sequence */
    OLED_WR_Byte(0xAEU, 0U); /* display off */
    OLED_WR_Byte(0x20U, 0U);
    OLED_WR_Byte(0x00U, 0U); /* horizontal addressing */
    OLED_WR_Byte(0x40U, 0U); /* start line 0 */
    OLED_WR_Byte(0xA1U, 0U); /* segment remap */
    OLED_WR_Byte(0xC8U, 0U); /* COM scan direction */
    OLED_WR_Byte(0xA8U, 0U);
    OLED_WR_Byte(0x3FU, 0U); /* multiplex 1/64 */
    OLED_WR_Byte(0xD3U, 0U);
    OLED_WR_Byte(0x00U, 0U); /* display offset */
    OLED_WR_Byte(0xD5U, 0U);
    OLED_WR_Byte(0x80U, 0U); /* clock divide */
    OLED_WR_Byte(0x81U, 0U);
    OLED_WR_Byte(0xCFU, 0U); /* contrast */
    OLED_WR_Byte(0xD9U, 0U);
    OLED_WR_Byte(0xF1U, 0U); /* pre-charge */
    OLED_WR_Byte(0xDAU, 0U);
    OLED_WR_Byte(0x12U, 0U); /* COM pins */
    OLED_WR_Byte(0xDBU, 0U);
    OLED_WR_Byte(0x40U, 0U); /* VCOM detect */
    OLED_WR_Byte(0x8DU, 0U);
    OLED_WR_Byte(0x14U, 0U); /* charge pump on */
    OLED_WR_Byte(0xA6U, 0U); /* normal display */
    OLED_WR_Byte(0xAFU, 0U); /* display on */

    OLED_Clear(0U);
    OLED_Refresh();
}

void OLED_Clear(uint8_t color)
{
    uint8_t page;
    uint8_t col;
    uint8_t fill = (color != 0U) ? 0xFFU : 0x00U;

    for (page = 0U; page < (OLED_HEIGHT / 8U); page++)
    {
        for (col = 0U; col < OLED_WIDTH; col++)
        {
            oled_buf[page][col] = fill;
        }
    }
}

void OLED_Refresh(void)
{
    uint8_t page;
    uint8_t col;

    for (page = 0U; page < (OLED_HEIGHT / 8U); page++)
    {
        OLED_SetPos(0U, page);
        for (col = 0U; col < OLED_WIDTH; col++)
        {
            OLED_WR_Byte(oled_buf[page][col], 1U);
        }
    }
}

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT))
    {
        return;
    }

    if (color != 0U)
    {
        oled_buf[y >> 3U][x] |= (uint8_t)(1U << (y & 7U));
    }
    else
    {
        oled_buf[y >> 3U][x] &= (uint8_t) ~(1U << (y & 7U));
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t scale)
{
    uint8_t i;
    uint8_t j;
    uint8_t px;
    uint8_t py;
    uint8_t col;
    uint8_t idx;

    if ((ch < 0x20) || (ch > 0x7E))
    {
        return;
    }
    if ((scale == 0U) || (scale > 2U))
    {
        scale = 1U;
    }
    if ((x + 6U * scale > OLED_WIDTH) || (y + 8U * scale > OLED_HEIGHT))
    {
        return;
    }

    idx = (uint8_t)(ch - 0x20);
    for (j = 0U; j < 5U; j++)
    {
        col = lcd_font_5x7[idx][j];
        for (i = 0U; i < 7U; i++)
        {
            if ((col & (1U << i)) == 0U)
            {
                continue;
            }
            for (px = 0U; px < scale; px++)
            {
                for (py = 0U; py < scale; py++)
                {
                    OLED_DrawPoint((uint8_t)(x + j * scale + px), (uint8_t)(y + i * scale + py),
                                   1U);
                }
            }
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, uint8_t scale, const char* str)
{
    while (*str != '\0')
    {
        OLED_ShowChar(x, y, *str++, scale);
        x = (uint8_t)(x + 6U * scale);
        if (x + 6U * scale > OLED_WIDTH)
        {
            x = 0U;
            y = (uint8_t)(y + 8U * scale);
        }
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t scale)
{
    uint8_t i;
    uint32_t divisor = 1U;

    if (len == 0U)
    {
        return;
    }

    for (i = 1U; i < len; i++)
    {
        divisor *= 10U;
    }

    for (i = 0U; i < len; i++)
    {
        uint8_t digit = (uint8_t)((num / divisor) % 10U);

        if (((num / divisor) == 0U) && (i < (uint8_t)(len - 1U)))
        {
            OLED_ShowChar(x, y, ' ', scale);
        }
        else
        {
            OLED_ShowChar(x, y, (char)('0' + digit), scale);
        }
        x = (uint8_t)(x + 6U * scale);
        divisor /= 10U;
    }
}
