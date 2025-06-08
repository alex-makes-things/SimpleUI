#include <stdint.h>
#include <pgmspace.h>
#include "gfxFrames.h"
#include <stdlib.h>
#include "gfxfont.h"
#include "glcdfont.c"
#include "WString.h"
#include "Print.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {
    #ifdef __AVR__
      return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
    #else
      // expression in __AVR__ section may generate "dereferencing type-punned
      // pointer will break strict-aliasing rules" warning In fact, on other
      // platforms (such as STM32) there is no need to do this pointer magic as
      // program memory may be read in a usual way So expression may be simplified
      return gfxFont->glyph + c;
    #endif //__AVR__
    }

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {
    #ifdef __AVR__
        return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
    #else
        // expression in __AVR__ section generates "dereferencing type-punned pointer
        // will break strict-aliasing rules" warning In fact, on other platforms (such
        // as STM32) there is no need to do this pointer magic as program memory may
        // be read in a usual way So expression may be simplified
        return gfxFont->bitmap;
    #endif //__AVR__
}

void Frame::cp437(bool x = true){
    _cp437 = x;
}

void Frame::charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {

    if (gfxFont) {

        if (c == '\n') { // Newline?
            *x = 0;        // Reset x to zero, advance y by one line
            *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if (c != '\r') { // Not a carriage return; is normal char
            uint8_t first = pgm_read_byte(&gfxFont->first),
            last = pgm_read_byte(&gfxFont->last);
            if ((c >= first) && (c <= last)) { // Char present in this font?
                GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t gw = pgm_read_byte(&glyph->width),
                gh = pgm_read_byte(&glyph->height),
                xa = pgm_read_byte(&glyph->xAdvance);
                int8_t xo = pgm_read_byte(&glyph->xOffset),
                yo = pgm_read_byte(&glyph->yOffset);
                if (wrap && ((*x + (((int16_t)xo + gw) * textsize_x)) > _width)) {
                    *x = 0; // Reset x to zero, advance y by one line
                    *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                int16_t tsx = (int16_t)textsize_x, tsy = (int16_t)textsize_y,
                x1 = *x + xo * tsx, y1 = *y + yo * tsy, x2 = x1 + gw * tsx - 1,
                y2 = y1 + gh * tsy - 1;
                if (x1 < *minx)
                *minx = x1;
                if (y1 < *miny)
                *miny = y1;
                if (x2 > *maxx)
                *maxx = x2;
                if (y2 > *maxy)
                *maxy = y2;
                *x += xa * tsx;
            }
        }

        } else { // Default font

            if (c == '\n') {        // Newline?
                *x = 0;               // Reset x to zero,
                *y += textsize_y * 8; // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
            } else if (c != '\r') { // Normal char; ignore carriage returns
                if (wrap && ((*x + textsize_x * 6) > _width)) { // Off right?
                    *x = 0;                                       // Reset x to zero,
                    *y += textsize_y * 8;                         // advance y one line
                }
                int x2 = *x + textsize_x * 6 - 1, // Lower-right pixel of char
                y2 = *y + textsize_y * 8 - 1;
                if (x2 > *maxx)
                    *maxx = x2; // Track max x, y
                if (y2 > *maxy)
                    *maxy = y2;
                if (*x < *minx)
                    *minx = *x; // Track min x, y
                if (*y < *miny)
                    *miny = *y;
                *x += textsize_x * 6; // Advance x one char
            }
        }
}

Frame::Frame(uint16_t *bitmap,uint16_t w,uint16_t h){
    _screenwidth = w;
    _screenheight = h;
    buffer = bitmap;
}

/**************************************************************************/
/*!
   @brief    Returns the current frame buffer as an array of 16-bit integers
*/
/**************************************************************************/
uint16_t* Frame::getFrame(){
    return buffer;
}

/**************************************************************************/
/*!
   @brief    Get the frame index of a determinate pixel
    @param    x  x coordinate
    @param    y  y coordinate
*/
/**************************************************************************/
uint16_t Frame::getPixelIndex(uint16_t x, uint16_t y){
    return y * _screenwidth + x;
}

/**************************************************************************/
/*!
   @brief    Draw a color in the frame to a determinate pixel
    @param    x  x coordinate
    @param    y  y coordinate
    @param    color 16 bit color
*/
/**************************************************************************/
void Frame::drawPixel(uint16_t x, uint16_t y, uint16_t color){
    buffer[getPixelIndex(x, y)] = color;
}

/**************************************************************************/
/*!
   @brief    Draw a line
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {

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

    for (; x0 <= x1; x0++) {
    if (steep) {
        drawPixel(y0, x0, color);
    } else {
        drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
        y0 += ystep;
        err += dx;
    }
    }
}

/**************************************************************************/
/*!
   @brief    Draw a perfectly vertical line (this is often optimized in a
   subclass!)
    @param    x   Top-most x coordinate
    @param    y   Top-most y coordinate
    @param    h   Height in pixels
   @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    drawLine(x, y, x, y + h - 1, color);
}

/**************************************************************************/
/*!
   @brief    Draw a perfectly horizontal line (this is often optimized in a
   subclass!)
    @param    x   Left-most x coordinate
    @param    y   Left-most y coordinate
    @param    w   Width in pixels
   @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    drawLine(x, y, x + w - 1, y, color);
}

/**************************************************************************/
/*!
   @brief   Draw a rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

/**************************************************************************/
/*!
   @brief    Fill a rectangle completely with one color. Update in subclasses if
   desired!
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
   @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t i = x; i < x + w; i++) {
        drawFastVLine(i, y, h, color);
    }
}

/**************************************************************************/
/*!
   @brief    Fill the screen completely with one color. Update in subclasses if
   desired!
    @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

/**************************************************************************/
/*!
   @brief    Draw a circle outline
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

/**************************************************************************/
/*!
    @brief    Quarter-circle drawer, used to do circles and roundrects
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    cornername  Mask bit #1 or bit #2 to indicate which quarters of
   the circle we're doing
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

/**************************************************************************/
/*!
    @brief  Quarter-circle drawer with fill, used for circles and roundrects
    @param  x0       Center-point x coordinate
    @param  y0       Center-point y coordinate
    @param  r        Radius of circle
    @param  corners  Mask bits indicating which quarters we're doing
    @param  delta    Offset from center-point, used for round-rects
    @param  color    16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++; // Avoid some +1's in the loop

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if (x < (y + 1)) {
            if (corners & 1)
                drawFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
            if (corners & 2)
                drawFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
        }
        if (y != py) {
            if (corners & 1)
                drawFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
            if (corners & 2)
                drawFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
            py = y;
        }
        px = x;
    }
}

/**************************************************************************/
/*!
   @brief    Draw a circle with filled color
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Frame::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

/**************************************************************************/
/*!
   @brief   Draw a triangle with no fill color
    @param    x0  Vertex #0 x coordinate
    @param    y0  Vertex #0 y coordinate
    @param    x1  Vertex #1 x coordinate
    @param    y1  Vertex #1 y coordinate
    @param    x2  Vertex #2 x coordinate
    @param    y2  Vertex #2 y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

/**************************************************************************/
/*!
   @brief     Draw a triangle with color-fill
    @param    x0  Vertex #0 x coordinate
    @param    y0  Vertex #0 y coordinate
    @param    x1  Vertex #1 x coordinate
    @param    y1  Vertex #1 y coordinate
    @param    x2  Vertex #2 x coordinate
    @param    y2  Vertex #2 y coordinate
    @param    color 16-bit 5-6-5 Color to fill/draw with
*/
/**************************************************************************/
void Frame::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
    if (x1 < a)
        a = x1;
    else if (x1 > b)
        b = x1;
    if (x2 < a)
        a = x2;
    else if (x2 > b)
        b = x2;
        drawFastHLine(a, y0, b - a + 1, color);
    return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
    dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);
            drawFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);
            drawFastHLine(a, y, b - a + 1, color);
    }
}

/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Frame::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    drawFastHLine(x + r, y, w - 2 * r, color);         // Top
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    drawFastVLine(x, y + r, h - 2 * r, color);         // Left
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color 16-bit 5-6-5 Color to draw/fill with
*/
/**************************************************************************/
void Frame::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    fillRect(x + r, y, w - 2 * r, h, color);
    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

/**************************************************************************/
/*!
   @brief   Draw a RAM-resident 16-bit image (RGB 5/6/5) at the specified (x,y)
   position. For 16-bit display devices; no color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void Frame::drawRGBBitmap(int16_t x, int16_t y, uint16_t bitmap[], int16_t w, int16_t h){
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
          drawPixel(x + i, y, bitmap[j * w + i]);
        }
      }
}

/**************************************************************************/
/*!
   @brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified
   (x,y) position. For 16-bit display devices; no color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void Frame::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            drawPixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
        }
    }
}

/**************************************************************************/
/*!
   @brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask
   (set bits = opaque, unset bits = clear) at the specified (x,y) position. BOTH
   buffers (color and mask) must be PROGMEM-resident. For 16-bit display
   devices; no color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    mask  byte array with monochrome mask bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void Frame::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) {
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t b = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                b <<= 1;
            else
                b = pgm_read_byte(&mask[j * bw + i / 8]);
            if (b & 0x80) {
                drawPixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
            }
        }
    }
}

/**************************************************************************/
/*!
   @brief   Draw a RAM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask (set
   bits = opaque, unset bits = clear) at the specified (x,y) position. BOTH
   buffers (color and mask) must be RAM-resident. For 16-bit display devices; no
   color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    mask  byte array with monochrome mask bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void Frame::drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h) {
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t b = 0;
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                b <<= 1;
            else
                b = mask[j * bw + i / 8];
            if (b & 0x80) {
                drawPixel(x + i, y, bitmap[j * w + i]);
            }
        }
    }
}

/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color,
   no background)
    @param    size_x  Font magnification level in X-axis, 1 is 'original' size
    @param    size_y  Font magnification level in Y-axis, 1 is 'original' size
*/
/**************************************************************************/
void Frame::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {

    if (!gfxFont) { // 'Classic' built-in font

        if ((x >= _width) ||              // Clip right
        (y >= _height) ||             // Clip bottom
        ((x + 6 * size_x - 1) < 0) || // Clip left
        ((y + 8 * size_y - 1) < 0))   // Clip top
        return;

        if (!_cp437 && (c >= 176))
            c++; // Handle 'classic' charset behavior

        for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for (int8_t j = 0; j < 8; j++, line >>= 1) {
                if (line & 1) {
                    if (size_x == 1 && size_y == 1)
                        drawPixel(x + i, y + j, color);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, color);
                    } else if (bg != color) {
                    if (size_x == 1 && size_y == 1)
                        drawPixel(x + i, y + j, bg);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
                    }
            }
        }
        if (bg != color) { // If opaque, draw vertical line for last column
            if (size_x == 1 && size_y == 1)
            drawFastVLine(x + 5, y, 8, bg);
            else
            fillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
        }

    } else { // Custom font

    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling
    // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
        int8_t xo = pgm_read_byte(&glyph->xOffset),
        yo = pgm_read_byte(&glyph->yOffset);
        uint8_t xx, yy, bits = 0, bit = 0;
        int16_t xo16 = 0, yo16 = 0;

        if (size_x > 1 || size_y > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if (bits & 0x80) {
                    if (size_x == 1 && size_y == 1) {
                        drawPixel(x + xo + xx, y + yo + yy, color);
                    } else {
                        fillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y,
                        size_x, size_y, color);
                    }
                }
                bits <<= 1;
            }
        }

    } // End classic vs custom font
}

/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color,
   no background)
    @param    size  Font magnification level, 1 is 'original' size
*/
/**************************************************************************/
void Frame::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    drawChar(x, y, c, color, bg, size, size);
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a string with current font/size.
            Pass string and a cursor position, returns UL corner and W,H.
    @param  str  The ASCII string to measure
    @param  x    The current cursor X
    @param  y    The current cursor Y
    @param  x1   The boundary X coordinate, returned by function
    @param  y1   The boundary Y coordinate, returned by function
    @param  w    The boundary width, returned by function
    @param  h    The boundary height, returned by function
*/
/**************************************************************************/
void Frame::getTextBounds(const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {

    uint8_t c; // Current character
    int16_t minx = 0x7FFF, miny = 0x7FFF, maxx = -1, maxy = -1; // Bound rect
    // Bound rect is intentionally initialized inverted, so 1st char sets it

    *x1 = x; // Initial position is value passed in
    *y1 = y;
    *w = *h = 0; // Initial size is zero

    while ((c = *str++)) {
        // charBounds() modifies x/y to advance for each character,
        // and min/max x/y are updated to incrementally build bounding rect.
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }

    if (maxx >= minx) {     // If legit string bounds were found...
        *x1 = minx;           // Update x1 to least X coord,
        *w = maxx - minx + 1; // And w to bound rect width
    }
    if (maxy >= miny) { // Same for height
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

/**************************************************************************/
/*!
    @brief    Helper to determine size of a string with current font/size. Pass
   string and a cursor position, returns UL corner and W,H.
    @param    str    The ascii string to measure (as an arduino String() class)
    @param    x      The current cursor X
    @param    y      The current cursor Y
    @param    x1     The boundary X coordinate, set by function
    @param    y1     The boundary Y coordinate, set by function
    @param    w      The boundary width, set by function
    @param    h      The boundary height, set by function
*/
/**************************************************************************/
void Frame::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w,uint16_t *h){
    if (str.length() != 0) {
        getTextBounds(const_cast<char *>(str.c_str()), x, y, x1, y1, w, h);
    }
}

/**************************************************************************/
/*!
    @brief    Helper to determine size of a PROGMEM string with current
   font/size. Pass string and a cursor position, returns UL corner and W,H.
    @param    str     The flash-memory ascii string to measure
    @param    x       The current cursor X
    @param    y       The current cursor Y
    @param    x1      The boundary X coordinate, set by function
    @param    y1      The boundary Y coordinate, set by function
    @param    w      The boundary width, set by function
    @param    h      The boundary height, set by function
*/
/**************************************************************************/
void Frame::getTextBounds(const __FlashStringHelper *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    uint8_t *s = (uint8_t *)str, c;

    *x1 = x;
    *y1 = y;
    *w = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while ((c = pgm_read_byte(s++)))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if (maxx >= minx) {
        *x1 = minx;
        *w = maxx - minx + 1;
    }
    if (maxy >= miny) {
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

/**************************************************************************/
/*!
    @brief   Set text 'magnification' size. Each increase in s makes 1 pixel
   that much bigger.
    @param  s  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
*/
/**************************************************************************/
void Frame::setTextSize(uint8_t s){
    setTextSize(s, s); 
}

/**************************************************************************/
/*!
    @brief   Set text 'magnification' size. Each increase in s makes 1 pixel
   that much bigger.
    @param  s_x  Desired text width magnification level in X-axis. 1 is default
    @param  s_y  Desired text width magnification level in Y-axis. 1 is default
*/
/**************************************************************************/
void Frame::setTextSize(uint8_t s_x, uint8_t s_y) {
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
  }

/**************************************************************************/
/*!
    @brief Set the font to display when print()ing, either custom or default
    @param  f  The GFXfont object, if NULL use built in 6x8 font
*/
/**************************************************************************/
void Frame::setFont(const GFXfont *f) {
    if (f) {          // Font struct pointer passed in?
      if (!gfxFont) { // And no current font struct?
        // Switching from classic to new font behavior.
        // Move cursor pos down 6 pixels so it's on baseline.
        cursor_y += 6;
      }
    } else if (gfxFont) { // NULL passed.  Current font struct defined?
      // Switching from new to classic font behavior.
      // Move cursor pos up 6 pixels so it's at top-left of char.
      cursor_y -= 6;
    }
    gfxFont = (GFXfont *)f;
}

/**********************************************************************/
  /*!
    @brief  Set text cursor location
    @param  x    X coordinate in pixels
    @param  y    Y coordinate in pixels
  */
  /**********************************************************************/
void Frame::setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

/**************************************************************************/
/*!
    @brief  Print one byte/character of data, used to support print()
    @param  c  The 8-bit ascii character to write
*/
/**************************************************************************/
size_t Frame::write(uint8_t c) {
    if (!gfxFont) { // 'Classic' built-in font
  
      if (c == '\n') {              // Newline?
        cursor_x = 0;               // Reset x to zero,
        cursor_y += textsize_y * 8; // advance y one line
      } else if (c != '\r') {       // Ignore carriage returns
        if (wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
          cursor_x = 0;                                       // Reset x to zero,
          cursor_y += textsize_y * 8; // advance y one line
        }
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                 textsize_y);
        cursor_x += textsize_x * 6; // Advance x one char
      }
  
    } else { // Custom font
  
      if (c == '\n') {
        cursor_x = 0;
        cursor_y +=
            (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
      } else if (c != '\r') {
        uint8_t first = pgm_read_byte(&gfxFont->first);
        if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
          GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
          uint8_t w = pgm_read_byte(&glyph->width),
                  h = pgm_read_byte(&glyph->height);
          if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
            int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
            if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
              cursor_x = 0;
              cursor_y += (int16_t)textsize_y *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                     textsize_y);
          }
          cursor_x +=
              (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
        }
      }
    }
    return 1;
  }
