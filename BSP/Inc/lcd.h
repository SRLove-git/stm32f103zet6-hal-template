/**
  ******************************************************************************
  * @file    lcd.h
  * @brief   TFTLCD driver (FSMC, ALIENTEK Elite board).
  *
  *          Interface on the Elite board:
  *            CS  - PG12 (FSMC_NE4)   | RS - PG0 (FSMC_A10)
  *            WR  - PD5  (FSMC_NWE)   | RD - PD4 (FSMC_NOE)
  *            D0..D15 - FSMC data bus (PD/PE pins)
  *            BL  - PB0 (backlight, high = on)
  *
  *          Memory mapping (16-bit bus, RS = A10):
  *            LCD_BASE = 0x6C000000 | 0x7FE
  *            LCD->LCD_REG writes command (A10=0)
  *            LCD->LCD_RAM writes data   (A10=1)
  *
  *          Preconfigured for ILI9341-family 2.8" modules (0x9341).
  *          Add other controllers by extending LCD_InitSequence().
  ******************************************************************************
  */

#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* FSMC-accessed LCD registers (see file header) */
typedef struct
{
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE ((uint32_t)(0x6C000000U | 0x000007FEU))
#define LCD      ((LCD_TypeDef *)LCD_BASE)

/* Common 16-bit RGB565 colors */
#define LCD_WHITE    0xFFFFU
#define LCD_BLACK    0x0000U
#define LCD_BLUE     0x001FU
#define LCD_RED      0xF800U
#define LCD_GREEN    0x07E0U
#define LCD_CYAN     0x07FFU
#define LCD_YELLOW   0xFFE0U
#define LCD_MAGENTA  0xF81FU
#define LCD_GRAY     0x7BEFU

/* Display directions */
#define LCD_DIR_L2R  0U  /* portrait  240 x 320 */
#define LCD_DIR_R2L  1U  /* landscape 320 x 240 */
#define LCD_DIR_U2D  2U  /* portrait flipped   */
#define LCD_DIR_D2U  3U  /* landscape flipped  */

extern uint16_t lcd_width;
extern uint16_t lcd_height;

void     LCD_Init(void);
uint16_t LCD_GetID(void);
void     LCD_SetDirection(uint8_t dir);

void     LCD_Clear(uint16_t color);
void     LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void     LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
uint16_t LCD_GetPoint(uint16_t x, uint16_t y);
void     LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
  * @brief Draw one ASCII character (5x7 bitmap, scaled).
  * @param ch  Character 0x20..0x7E.
  * @param scale 1..4 (font cell = 5*scale x 8*scale pixels).
  */
void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint8_t scale,
                  uint16_t color, uint16_t bg);

void LCD_ShowString(uint16_t x, uint16_t y, uint8_t scale, const char *str,
                    uint16_t color, uint16_t bg);

/**
  * @brief Show a decimal number, right-aligned to len digits
  *        (leading positions are left blank).
  */
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t scale,
                 uint16_t color, uint16_t bg);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */
