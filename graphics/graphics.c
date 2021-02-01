/******************************************************************************

    (c) Copyright 2019 ee-quipment.com

    graphics.c - graphics and font handling.

 *****************************************************************************/


#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "graphics.h"


#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#define GFX_BM_PHYSICAL_WIDTH(width)    (32 * ((width + 31) / 32))


void gfx_drawPixel(gfx_bm_t *bm, int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= bm->w) || (y < 0) || (y >= bm->h))
      return;

      switch (color)
      {
        case GFX_COLOR_WHITE:   bm->bm8[x+ (y/8)*bm->w] |=  (1 << (y&7)); break;
        case GFX_COLOR_BLACK:   bm->bm8[x+ (y/8)*bm->w] &= ~(1 << (y&7)); break;
      }

  }

void gfx_drawLine(gfx_bm_t *bm, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            gfx_drawPixel(bm, y0, x0, color);
        } else {
            gfx_drawPixel(bm, x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void _gfx_drawCircleHelper(gfx_bm_t *bm, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            gfx_drawPixel(bm, x0 + x, y0 + y, color);
            gfx_drawPixel(bm, x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            gfx_drawPixel(bm, x0 + x, y0 - y, color);
            gfx_drawPixel(bm, x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            gfx_drawPixel(bm, x0 - y, y0 + x, color);
            gfx_drawPixel(bm, x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            gfx_drawPixel(bm, x0 - y, y0 - x, color);
            gfx_drawPixel(bm, x0 - x, y0 - y, color);
        }
    }
}

void _gfx_fillCircleHelper(gfx_bm_t *bm, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            gfx_drawLine(bm, x0+x, y0-y, x0+x, y0-y+2*y+delta, color);
            gfx_drawLine(bm, x0+y, y0-x, x0+y, y0-x+2*x+delta, color);
        }
        if (cornername & 0x2) {
            gfx_drawLine(bm, x0-x, y0-y, x0-x, y0-y+2*y+delta, color);
            gfx_drawLine(bm, x0-y, y0-x, x0-y, y0-x+2*x+delta, color);
        }
    }
}

void gfx_drawRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    gfx_drawLine(bm, x, y, x+w-1, y, color);
    gfx_drawLine(bm, x, y+h-1, x+w-1, y+h-1, color);
    gfx_drawLine(bm, x, y, x, y+h-1, color);
    gfx_drawLine(bm, x+w-1, y, x+w-1, y+h-1, color);
}

void gfx_drawRoundRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    gfx_drawLine(bm, x+r  , y    , x+r+w-2*r,   y,         color); // Top
    gfx_drawLine(bm, x+r  , y+h-1, x+r+w-2*r,   y+h-1,     color); // Bottom
    gfx_drawLine(bm, x    , y+r  , x,           y+r+h-2*r, color); // Left
    gfx_drawLine(bm, x+w-1, y+r  , x+w-1,       y+r+h-2*r, color); // Right
    _gfx_drawCircleHelper(bm, x+r    , y+r    , r, 1, color);
    _gfx_drawCircleHelper(bm, x+w-r-1, y+r    , r, 2, color);
    _gfx_drawCircleHelper(bm, x+w-r-1, y+h-r-1, r, 4, color);
    _gfx_drawCircleHelper(bm, x+r    , y+h-r-1, r, 8, color);
}

void gfx_fillRoundRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    gfx_fillRect(bm, x+r, y, w-2*r, h, color);
    _gfx_fillCircleHelper(bm, x+w-r-1, y+r, r, 1, h-2*r-1, color);
    _gfx_fillCircleHelper(bm, x+r    , y+r, r, 2, h-2*r-1, color);
}


void gfx_fillRect(gfx_bm_t *bm, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t i=y; i<y+h; i++) {
        gfx_drawLine(bm, x, i, x+w-1, i, color);
    }
}

void gfx_bitBLT(gfx_bm_t *bm, int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            if(i & 7) byte <<= 1;
            else      byte   = bitmap[j * byteWidth + i / 8];
            if(byte & 0x80) gfx_drawPixel(bm, x+i, y, color);
        }
    }
}

void gfx_clear(gfx_bm_t *bm) {
  memset(bm->bm8, 0, (bm->h*GFX_BM_PHYSICAL_WIDTH(bm->w)/8));
}

/*
 *  Character and text handling
 */

void gfx_setFont(gfx_bm_t *bm, const gfx_font_t *font) {
    bm->font = font;
}

void gfx_setCursor(gfx_bm_t *bm, int16_t x, int16_t y) {
    bm->cursor.x = x;
    bm->cursor.y = y;
}

void gfx_moveCursor(gfx_bm_t *bm, int16_t dx, int16_t dy) {
    gfx_setCursor(bm, bm->cursor.x + dx, bm->cursor.y + dy);
}

void gfx_drawChar(gfx_bm_t *bm, int16_t x, int16_t y, uint16_t color, char c) {
    gfx_glyph_t *glyph;
    uint8_t     *f_bitmap;

    if ((c < bm->font->first) || (c > bm->font->last)) { return; }

    c -= bm->font->first;   // offset c to first glyph in font file
    glyph = &(bm->font->glyph[c]);

    uint16_t bo = glyph->bitmapOffset;
    uint8_t  w  = glyph->width,
             h  = glyph->height;
    int8_t   xo = glyph->xOffset,
             yo = glyph->yOffset;
    uint8_t  xx, yy, bits = 0, bit = 0;

    // is there an associated bitmap?
    if ((w == 0) || (h ==0)) { return; }

    f_bitmap = bm->font->bitmap;

    for(yy=0; yy<h; yy++) {
        for(xx=0; xx<w; xx++) {
            if(!(bit++ & 7)) {
                bits = f_bitmap[bo++];
            }
            if(bits & 0x80) {
                gfx_drawPixel(bm, x+xo+xx, y+yo+yy, color);
            }
            bits <<= 1;
        }
    }
}

void gfx_write(gfx_bm_t *bm, const uint16_t color, const char *str) {
    gfx_glyph_t *glyph;

    while (*str) {
        char c = *str++;

        if (c == '\n') {
            bm->cursor.x  = 0;
            bm->cursor.y += bm->font->yAdvance;
        }
        if ((c < bm->font->first) || (c > bm->font->last)) { continue; }

        glyph = &(bm->font->glyph[c - bm->font->first]);
        gfx_drawChar(bm, bm->cursor.x, bm->cursor.y, color, c);
        bm->cursor.x += glyph->xAdvance;
    }
}








