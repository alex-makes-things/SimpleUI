#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "nicerlandscape.h"
#include <scaler.h>
#include <images/home_images.h>

#define SDA 21
#define SCL 22
#define DC 23
#define RST 17
#define SCREENHEIGHT 64
#define SCREENWIDTH 128

SPIClass spi = SPIClass(VSPI);
Adafruit_ST7735 tft = Adafruit_ST7735(&spi, -1, DC, RST);
GFXcanvas16 canvas = GFXcanvas16(128,64);


//-----------FUNCTION PROTOTYPES----------------//

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
float mapM(float x, float in_min, float in_max, float out_min, float out_max);
void loadingBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t startTime, uint16_t duration, uint16_t color);
int lerp(float v0, float v1, float t);
float lerpF(float v0, float v1, float t);
float clamp(float n, float min, float max);
void renderBmp8(int x, int y, const uint8_t *image, int w, int h, float scaling, uint16_t color);

//-----------FUNCTION PROTOTYPES----------------//


void setup() {
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
}

//-------------BEFORE LOOP----------------//
uint64_t start, lerptime = millis();
uint64_t pauseT = 0;
static bool isDone = false;
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = false;
static float scaling_factor = 0.7;
//-------------SETTINGS----------------//


void loop() {
    //canvas.drawRGBBitmap(0,0,nicerlandscape,SCREENWIDTH,SCREENHEIGHT);
    canvas.fillScreen(0x0000);
    if(render_frametime){
      canvas.setCursor(0,48);
      canvas.setTextSize(2);
      canvas.setTextColor(ST7735_GREEN);
      canvas.setTextWrap(false);
      canvas.print(millis()-start);
    }
    
    renderBmp8(46,14,home_large_test, HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, scaling_factor, 0xffff);
    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT);
    start = millis();
}  






/**************************************************************************/
/*!
   @brief      Draw a scaled monochrome bitmap to the universal canvas
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    image  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
    @param    scaling Scaling factor of the bitmap
    @param    color 16-bit color with which the bitmap should be drawn
*/
/**************************************************************************/
void renderBmp8(int x, int y, const uint8_t *image, int w, int h, float scaling, uint16_t color){
  uint8_t* output = new uint8_t[getArrSize8(w, h, scaling)];
  pair<unsigned int, unsigned int> imagesize = scale(image, w, h, output, scaling);
  canvas.drawBitmap(x,y, output, imagesize.first, imagesize.second, color);
  delete[] output;
}
/**************************************************************************/
/*!
   @brief      A blazingly fast method for drawing an RGB bitmap to the screen
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  tft.startWrite();
  tft.setAddrWindow(x,y,w,h);
  tft.writePixels(bitmap, w*h, false);
  tft.endWrite();
}
float mapM(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void loadingBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t startTime, uint16_t duration, uint16_t color){
  canvas.drawRect(x, y, w, h, color);
  int load_progress = micros() - startTime;  
  if(load_progress < duration*1000){
    float progress = (float)load_progress / (duration * 1000);  // Smooth 0 to 1 scaling
    int bar_width = (int)(progress * (float)w);  // Scale to bar size
    canvas.fillRect(x, y, bar_width, h, color);
  }
}
int lerp(float v0, float v1, float t) {
  return int(round((1 - t) * v0 + t * v1));
}
float lerpF(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}
float clamp(float n, float min, float max) {
  return std::max(min, std::min(n, max));
}
