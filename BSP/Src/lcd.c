/**
 ******************************************************************************
 * @file    lcd.c
 * @brief   TFTLCD driver via FSMC (ILI9341-family).
 ******************************************************************************
 */

#include "lcd.h"
#include "lcdfont.h"
#include "stm32f1xx_ll_fsmc.h"

uint16_t lcd_width = 240U;
uint16_t lcd_height = 320U;

static void LCD_WR_REG(uint16_t reg)
{
    LCD->LCD_REG = reg;
}

static void LCD_WR_DATA(uint16_t data)
{
    LCD->LCD_RAM = data;
}

static uint16_t LCD_RD_DATA(void)
{
    return LCD->LCD_RAM;
}

/**
 * @brief Set the GRAM write window.
 */
static void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    LCD_WR_REG(0x2AU);
    LCD_WR_DATA((uint16_t)(x0 >> 8U));
    LCD_WR_DATA((uint16_t)(x0 & 0xFFU));
    LCD_WR_DATA((uint16_t)(x1 >> 8U));
    LCD_WR_DATA((uint16_t)(x1 & 0xFFU));

    LCD_WR_REG(0x2BU);
    LCD_WR_DATA((uint16_t)(y0 >> 8U));
    LCD_WR_DATA((uint16_t)(y0 & 0xFFU));
    LCD_WR_DATA((uint16_t)(y1 >> 8U));
    LCD_WR_DATA((uint16_t)(y1 & 0xFFU));

    LCD_WR_REG(0x2CU); /* write memory */
}

/**
 * @brief Configure the FSMC Bank1 NE4 controller for the LCD.
 */
static void LCD_FSMC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    FSMC_NORSRAM_InitTypeDef fsmc = {0};
    FSMC_NORSRAM_TimingTypeDef timing = {0};

    __HAL_RCC_FSMC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* FSMC pins: D0..D15, NOE (RD), NWE (WR), A10 (RS), NE4 (CS) */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 |
                          GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                          GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_12; /* A10, NE4 */
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* Backlight: PB0, high = on (full brightness; PWM later if needed) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    /* Timing (2.8" ILI9341 @ 72 MHz HCLK; see RM0008 FSMC section) */
    timing.AddressSetupTime = 1U;
    timing.AddressHoldTime = 0U;
    timing.DataSetupTime = 3U; /* ALIENTEK-standard timing for 2.8" ILI9341 */
    timing.BusTurnAroundDuration = 0U;
    timing.CLKDivision = 0U;
    timing.DataLatency = 0U;
    timing.AccessMode = FSMC_ACCESS_MODE_A;

    fsmc.NSBank = FSMC_NORSRAM_BANK4;
    fsmc.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    fsmc.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    fsmc.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    fsmc.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    fsmc.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    fsmc.WrapMode = FSMC_WRAP_MODE_DISABLE;
    fsmc.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    fsmc.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    fsmc.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    fsmc.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    fsmc.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    fsmc.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    fsmc.PageSize = FSMC_PAGE_SIZE_NONE;

    if ((FSMC_NORSRAM_Init(FSMC_NORSRAM_DEVICE, &fsmc) != HAL_OK) ||
        (FSMC_NORSRAM_Timing_Init(FSMC_NORSRAM_DEVICE, &timing, FSMC_NORSRAM_BANK4) != HAL_OK) ||
        (FSMC_NORSRAM_WriteOperation_Enable(FSMC_NORSRAM_DEVICE, FSMC_NORSRAM_BANK4) != HAL_OK))
    {
        Error_Handler();
    }

    /* The F1 LL driver clears MBKEN during init and never re-enables it;
     * without this, accesses to the NE4 region trigger a bus fault. */
    __FSMC_NORSRAM_ENABLE(FSMC_NORSRAM_DEVICE, FSMC_NORSRAM_BANK4);
}

/**
 * @brief ILI9341-family power-on init sequence.
 */
static void LCD_InitSequence(void)
{
    HAL_Delay(50U);

    LCD_WR_REG(0x01U); /* software reset */
    HAL_Delay(120U);

    LCD_WR_REG(0xCFU);
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0xC1U);
    LCD_WR_DATA(0x30U);

    LCD_WR_REG(0xEDU);
    LCD_WR_DATA(0x64U);
    LCD_WR_DATA(0x03U);
    LCD_WR_DATA(0x12U);
    LCD_WR_DATA(0x81U);

    LCD_WR_REG(0xE8U);
    LCD_WR_DATA(0x85U);
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0x78U);

    LCD_WR_REG(0xCBU);
    LCD_WR_DATA(0x39U);
    LCD_WR_DATA(0x2CU);
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0x34U);
    LCD_WR_DATA(0x02U);

    LCD_WR_REG(0xF7U);
    LCD_WR_DATA(0x20U);

    LCD_WR_REG(0xEAU);
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0x00U);

    LCD_WR_REG(0xC0U); /* power control 1 */
    LCD_WR_DATA(0x23U);

    LCD_WR_REG(0xC1U); /* power control 2 */
    LCD_WR_DATA(0x10U);

    LCD_WR_REG(0xC5U); /* vcom control 1 */
    LCD_WR_DATA(0x3EU);
    LCD_WR_DATA(0x28U);

    LCD_WR_REG(0xC7U); /* vcom control 2 */
    LCD_WR_DATA(0x86U);

    LCD_WR_REG(0x3AU); /* pixel format: 16 bpp */
    LCD_WR_DATA(0x55U);

    LCD_WR_REG(0xB1U); /* frame rate */
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0x18U);

    LCD_WR_REG(0xB6U); /* display function control */
    LCD_WR_DATA(0x08U);
    LCD_WR_DATA(0x82U);
    LCD_WR_DATA(0x27U);

    LCD_WR_REG(0xF2U); /* 3 gamma control disable */
    LCD_WR_DATA(0x00U);

    LCD_WR_REG(0x26U); /* gamma curve 1 */
    LCD_WR_DATA(0x01U);

    LCD_WR_REG(0xE0U); /* positive gamma */
    LCD_WR_DATA(0x0FU);
    LCD_WR_DATA(0x31U);
    LCD_WR_DATA(0x2BU);
    LCD_WR_DATA(0x0CU);
    LCD_WR_DATA(0x0EU);
    LCD_WR_DATA(0x08U);
    LCD_WR_DATA(0x4EU);
    LCD_WR_DATA(0xF1U);
    LCD_WR_DATA(0x37U);
    LCD_WR_DATA(0x07U);
    LCD_WR_DATA(0x10U);
    LCD_WR_DATA(0x03U);
    LCD_WR_DATA(0x0EU);
    LCD_WR_DATA(0x09U);
    LCD_WR_DATA(0x00U);

    LCD_WR_REG(0xE1U); /* negative gamma */
    LCD_WR_DATA(0x00U);
    LCD_WR_DATA(0x0EU);
    LCD_WR_DATA(0x14U);
    LCD_WR_DATA(0x03U);
    LCD_WR_DATA(0x11U);
    LCD_WR_DATA(0x07U);
    LCD_WR_DATA(0x31U);
    LCD_WR_DATA(0xC1U);
    LCD_WR_DATA(0x48U);
    LCD_WR_DATA(0x08U);
    LCD_WR_DATA(0x0FU);
    LCD_WR_DATA(0x0CU);
    LCD_WR_DATA(0x31U);
    LCD_WR_DATA(0x36U);
    LCD_WR_DATA(0x0FU);

    LCD_WR_REG(0x11U); /* sleep out */
    HAL_Delay(120U);

    LCD_WR_REG(0x29U); /* display on */
    HAL_Delay(20U);
}

void LCD_Init(void)
{
    LCD_FSMC_Init();
    LCD_InitSequence();

    (void)LCD_GetID(); /* ID not strictly needed for init */
    LCD_SetDirection(LCD_DIR_L2R);
    LCD_Clear(LCD_WHITE);
}

uint16_t LCD_GetID(void)
{
    uint16_t id;

    LCD_WR_REG(0xD3U);   /* read ID4 for ILI9341 */
    (void)LCD_RD_DATA(); /* dummy read */
    id = (uint16_t)((uint16_t)LCD_RD_DATA() << 8U);
    id |= (uint16_t)LCD_RD_DATA();

    return id; /* 0x9341 for ILI9341 */
}

void LCD_SetDirection(uint8_t dir)
{
    static const uint8_t madctl[4] = {0x48U, 0x28U, 0x88U, 0xE8U};
    static const uint16_t width[4] = {240U, 320U, 240U, 320U};
    static const uint16_t height[4] = {320U, 240U, 320U, 240U};

    if (dir > 3U)
    {
        dir = 0U;
    }

    lcd_width = width[dir];
    lcd_height = height[dir];

    LCD_WR_REG(0x36U); /* MADCTL */
    LCD_WR_DATA(madctl[dir]);
}

void LCD_Clear(uint16_t color)
{
    uint32_t n = (uint32_t)lcd_width * (uint32_t)lcd_height;

    LCD_SetWindow(0U, 0U, (uint16_t)(lcd_width - 1U), (uint16_t)(lcd_height - 1U));
    while (n-- > 0U)
    {
        LCD_WR_DATA(color);
    }
}

void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint32_t n;

    if ((x1 < x0) || (y1 < y0))
    {
        return;
    }

    n = (uint32_t)(x1 - x0 + 1U) * (uint32_t)(y1 - y0 + 1U);
    LCD_SetWindow(x0, y0, x1, y1);
    while (n-- > 0U)
    {
        LCD_WR_DATA(color);
    }
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetWindow(x, y, x, y);
    LCD_WR_DATA(color);
}

uint16_t LCD_GetPoint(uint16_t x, uint16_t y)
{
    uint16_t color;

    LCD_SetWindow(x, y, x, y);
    LCD_WR_REG(0x2EU);   /* read memory */
    (void)LCD_RD_DATA(); /* dummy read (ILI9341) */
    color = LCD_RD_DATA();

    return color;
}

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx, dy, sx, sy, err, e2;
    int16_t ix = (int16_t)x0, iy = (int16_t)y0;
    int16_t tx = (int16_t)x1, ty = (int16_t)y1;

    dx = (ix < tx) ? (tx - ix) : (ix - tx);
    dy = (iy < ty) ? (iy - ty) : (ty - iy); /* negated: dy = -|dy| */
    dy = (int16_t)(0 - dy);
    sx = (ix < tx) ? 1 : -1;
    sy = (iy < ty) ? 1 : -1;
    err = (int16_t)(dx + dy);

    for (;;)
    {
        LCD_DrawPoint((uint16_t)ix, (uint16_t)iy, color);
        if ((ix == tx) && (iy == ty))
        {
            break;
        }
        e2 = (int16_t)(2 * err);
        if (e2 >= dy)
        {
            err = (int16_t)(err + dy);
            ix = (int16_t)(ix + sx);
        }
        if (e2 <= dx)
        {
            err = (int16_t)(err + dx);
            iy = (int16_t)(iy + sy);
        }
    }
}

void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint8_t scale, uint16_t color, uint16_t bg)
{
    uint8_t i, j;
    uint8_t idx;
    uint8_t col;

    if ((ch < 0x20) || (ch > 0x7E))
    {
        return;
    }
    if ((scale == 0U) || (scale > 4U))
    {
        scale = 1U;
    }

    idx = (uint8_t)(ch - 0x20);
    for (j = 0U; j < 5U; j++)
    {
        col = lcd_font_5x7[idx][j];
        for (i = 0U; i < 8U; i++)
        {
            uint16_t px = (uint16_t)(x + j * scale);
            uint16_t py = (uint16_t)(y + i * scale);
            LCD_Fill(px, py, (uint16_t)(px + scale - 1U), (uint16_t)(py + scale - 1U),
                     (col & (1U << i)) ? color : bg);
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, uint8_t scale, const char* str, uint16_t color,
                    uint16_t bg)
{
    while (*str != '\0')
    {
        LCD_ShowChar(x, y, *str++, scale, color, bg);
        x = (uint16_t)(x + 6U * scale);
        if ((uint16_t)(x + 6U * scale) > lcd_width)
        {
            x = 0U;
            y = (uint16_t)(y + 8U * scale);
        }
    }
}

void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t scale, uint16_t color,
                 uint16_t bg)
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
            LCD_ShowChar(x, y, ' ', scale, color, bg);
        }
        else
        {
            LCD_ShowChar(x, y, (char)('0' + digit), scale, color, bg);
        }
        x = (uint16_t)(x + 6U * scale);
        divisor /= 10U;
    }
}
