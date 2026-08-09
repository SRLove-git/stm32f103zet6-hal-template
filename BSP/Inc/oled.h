/**
 ******************************************************************************
 * @file    oled.h
 * @brief   ATK-OLED 0.96" SSD1306 driver (8080 parallel mode).
 *
 *          The OLED module plugs into the P4 OLED/CAMERA socket, left-aligned.
 *          Interface pins on the Elite board (measured from the manual):
 *            DC  - PD3 (OV_SCL) | CS - PD6 (FIFO_WRST)
 *            WR  - PG14 (OV_RRST) | RD - PG13 (OV_SDA, held high)
 *            RST - PG15 (OV_OE) | D0..D7 - PC0..PC7 (OV_D0..D7)
 *
 *          The module must be set to 8080 parallel mode (default BS1/BS2
 *          solder bridges). 128x64 resolution, 8 pages of 128 bytes.
 ******************************************************************************
 */

#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define OLED_WIDTH 128U
#define OLED_HEIGHT 64U

    void OLED_Init(void);

    /**
     * @brief Fill the display buffer (0 = off, 0xFF = on).
     * @note  Call OLED_Refresh() afterwards to update the panel.
     */
    void OLED_Clear(uint8_t color);

    /**
     * @brief Upload the display buffer to the SSD1306.
     */
    void OLED_Refresh(void);

    void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);

    /**
     * @brief Draw one ASCII character (5x7 font, scaled) into the buffer.
     * @param scale 1..2 (font cell = 5*scale x 8*scale pixels).
     */
    void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t scale);

    void OLED_ShowString(uint8_t x, uint8_t y, uint8_t scale, const char* str);

    /**
     * @brief Show a decimal number, right-aligned to len digits.
     */
    void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t scale);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
