/**
  * @file    lcdfont.h
  * @brief   5x7 fixed-width ASCII bitmap font (glyphs 0x20..0x7E).
  */

#ifndef __LCDFONT_H
#define __LCDFONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 95 glyphs x 5 columns, LSB = top row */
extern const uint8_t lcd_font_5x7[95][5];

#ifdef __cplusplus
}
#endif

#endif /* __LCDFONT_H */
