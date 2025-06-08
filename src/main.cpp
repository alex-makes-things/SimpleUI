#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "nicerlandscape.h"
#include "test.h"

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
float clamp(float n, float min, float max);
//-----------FUNCTION PROTOTYPES----------------//


void setup() {
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
}

//-------------BEFORE LOOP----------------//

uint64_t rect_c, start= millis();
bool render_frametime = false;
int frametime = 0;
char string_buffer[3];
const unsigned int interval = 500;
const unsigned int duration = 5000;

//-------------BEFORE LOOP----------------//
//-------------SETTINGS----------------//

const int init_square_size = 20;
const int f_square_size = 50;
int square_size = init_square_size;
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
    if(square_size < f_square_size){
      square_size = clamp(lerp(init_square_size, f_square_size, mapM(millis()-rect_c, 0, duration, 0, 1)), init_square_size, f_square_size);
      
    }else{
      square_size = init_square_size;
      rect_c = millis();
    }
    
    canvas.drawRect(64-round((square_size/2)),32-round((square_size/2)),square_size,square_size, ST7735_RED);

    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT);
}  



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
float clamp(float n, float min, float max) {
  return std::max(min, std::min(n, max));
}
