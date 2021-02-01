/******************************************************************************

    (c) Copyright 2019 ee-quipment.com

    graphics.h - graphics and font handling.

 *****************************************************************************/


#ifndef _graphics_H_
#define _graphics_H_

#include <stdint.h>

#define GFX_COLOR_BLACK   0
#define GFX_COLOR_WHITE   1


typedef struct gfx_glyph_t {  // Data stored PER GLYPH
  uint16_t bitmapOffset;      // Pointer into GFXfont->bitmap
  uint8_t  width, height;     // Bitmap dimensions in pixels
  uint8_t  xAdvance;          // Distance to advance cursor (x axis)
  int8_t   xOffset, yOffset;  // Dist from cursor pos to UL corner
} gfx_glyph_t;


typedef struct gfx_font_t {   // Data stored for FONT AS A WHOLE:
  uint8_t     *bitmap;        // Glyph bitmaps, concatenated
  gfx_glyph_t *glyph;         // Glyph array
  uint8_t      first, last;   // ASCII extents
  uint8_t      yAdvance;      // Newline distance (y axis)
} gfx_font_t;


typedef struct gfx_bm_t {
    int16_t            w, h;
    union {
        uint32_t      *bm32;
        uint8_t       *bm8;
    };
    const gfx_font_t  *font;
    struct {
        int16_t        x, y;
    } cursor;
} gfx_bm_t;


// Macro to define the storage for a bitmap. Name has global scope.
// Round width up to a multiple of 4 bytes to allow 32 bit operations.
#define NEW_BITMAP(name, width, height)                                 \
  uint32_t name##_bm32[((width+31) / 32) * height];                     \
  gfx_bm_t name##_struct = { width, height, name##_bm32, NULL, 0, 0 };  \
  gfx_bm_t * const name = &name##_struct


void gfx_clear(gfx_bm_t *bm);
void gfx_drawPixel(gfx_bm_t *bm, int16_t x, int16_t y, uint16_t color);
void gfx_drawLine(gfx_bm_t *bm, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void gfx_drawRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gfx_fillRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gfx_drawRoundRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void gfx_fillRoundRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void gfx_bitBLT(gfx_bm_t *bm, int16_t x, int16_t y, uint8_t *src_bm, int16_t w, int16_t h, uint16_t color);

void gfx_setFont(gfx_bm_t *bm, const gfx_font_t *font);
void gfx_setCursor(gfx_bm_t *bm, int16_t x, int16_t y);
void gfx_moveCursor(gfx_bm_t *bm, int16_t dx, int16_t dy);
void gfx_drawChar(gfx_bm_t *bm, int16_t x, int16_t y, uint16_t color, char c);
void gfx_write(gfx_bm_t *bm, const uint16_t color, const char *str);





#endif  /* _graphics_H_ */


