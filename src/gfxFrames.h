#include <stdint.h>
#include <pgmspace.h>
#include "gfxfont.h"
#include "WString.h"
#include "Print.h"

class Frame : public Print{
public:
    Frame(uint16_t *bitmap,uint16_t w,uint16_t h);
    uint16_t* getFrame();
    uint16_t getPixelIndex(uint16_t x, uint16_t y);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void fillScreen(uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t bitmap[], int16_t w, int16_t h);
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
    void cp437(bool x);
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void setTextSize(uint8_t s);
    void setTextSize(uint8_t sx, uint8_t sy);
    void setFont(const GFXfont *f = NULL);
    using Print::write;
    #if ARDUINO >= 100
    virtual size_t write(uint8_t);
    #else
    virtual void write(uint8_t);
    #endif

    /**********************************************************************/
  /*!
    @brief  Set text cursor location
    @param  x    X coordinate in pixels
    @param  y    Y coordinate in pixels
  */
  /**********************************************************************/
    void setCursor(int16_t x, int16_t y);
    /**********************************************************************/
  /*!
    @brief   Set text font color with transparant background
    @param   c   16-bit 5-6-5 Color to draw text with
    @note    For 'transparent' background, background and foreground
             are set to same color rather than using a separate flag.
  */
  /**********************************************************************/
    void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }
  /**********************************************************************/
  /*!
    @brief   Set text font color with custom background color
    @param   c   16-bit 5-6-5 Color to draw text with
    @param   bg  16-bit 5-6-5 Color to draw background/fill with
  */
  /**********************************************************************/ 
  void setTextColor(uint16_t c, uint16_t bg) {
    textcolor = c;
    textbgcolor = bg;
  }

  /**********************************************************************/
  /*!
  @brief  Set whether text that is too long for the screen width should
          automatically wrap around to the next line (else clip right).
  @param  w  true for wrapping, false for clipping
  */
  /**********************************************************************/
  void setTextWrap(bool w) { wrap = w; }

private:
    void charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
    int16_t WIDTH;        ///< This is the 'raw' display width - never changes
    int16_t HEIGHT;       ///< This is the 'raw' display height - never changes
    int16_t _width;       ///< Display width as modified by current rotation
    int16_t _height;      ///< Display height as modified by current rotation
    int16_t cursor_x;     ///< x location to start print()ing text
    int16_t cursor_y;     ///< y location to start print()ing text
    uint16_t textcolor;   ///< 16-bit background color for print()
    uint16_t textbgcolor; ///< 16-bit text color for print()
    uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
    uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
    uint8_t rotation;     ///< Display rotation (0 thru 3)
    uint16_t _screenwidth;
    uint16_t _screenheight;
    uint16_t *buffer;
    GFXfont *gfxFont;
    bool wrap;            ///< If set, 'wrap' text at right edge of display
    bool _cp437;        ///< If set, use correct CP437 charset (default is off)
};

