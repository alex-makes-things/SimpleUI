#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "nicerlandscape.h"
#include <ImageAssist.h>
#include <images/home_images.h>

#define SDA 21
#define SCL 22
#define DC 23
#define RST 17
#define SCREENHEIGHT 64
#define SCREENWIDTH 128
const float EPSILON = 0.01; //The epsilon is needed because of floating point precision, and we need a tolerance margin, so sometimes "==" wouldn't work



SPIClass spi = SPIClass(VSPI);
Adafruit_ST7735 tft = Adafruit_ST7735(&spi, -1, DC, RST);
GFXcanvas16 canvas = GFXcanvas16(128,64);


//-----------FUNCTION PROTOTYPES----------------//

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color);

//-----------FUNCTION PROTOTYPES----------------//

struct Animator{
  bool isDone = false;   //Signals if the animation is complete or not
  bool looping = false;  //Makes the animation loop
  bool reverse = false;  //"Inverts" the final and initial values, (just in the calculations)
  unsigned int duration; //How long the animation takes to go from initial to final and viceversa
  float initial;
  float final;
  float progress = (reverse) ? final : initial;  //The progress is ultimately the output of the structure, which lies clamped in between initial and final
  uint64_t currentTime = millis();   //This is needed for the temporal aspect of the interpolation
  Animator(float i, float f, unsigned int d, bool loop){  //Basic constructor
    duration = d;
    initial = i;
    final = f;
    looping = loop;
  }
  void update(){
    if(reverse)  //Not the most efficient way, but this makes sure that we execute the right code whether the animation is reversed or not
    {
      if(fabs(progress-initial)>=EPSILON && isDone == false){
        progress = clamp(lerpF(initial, final, fabs(1-mapF(millis()-currentTime, 0, duration, 0, 1))), initial, final);
      }else if(isDone==false && fabs(progress - initial) < EPSILON)  //Declares the animation as done and rounds the progress to the final amount
      {
        isDone = true;
        progress = initial;
      }
    }
    else{
      if(fabs(progress-final)>=EPSILON && isDone == false){  
        progress = clamp(lerpF(initial, final, mapF(millis()-currentTime, 0, duration, 0, 1)), initial, final);  
      }else if(isDone==false && fabs(progress - final) < EPSILON)
      {
        isDone = true;
        progress = final;
      }
    }

    if(isDone && looping){  //Independent check that loops the animation by resetting the done status, progress, and internal timing.
      isDone = false;
      progress = (reverse) ? final : initial;
      currentTime = millis();
    }
  }

  void invert(){ //Function that can be called at runtime which inverts the direction of the interpolation
    //WORK IN PROGRESS
    reverse = !reverse;
  }
};
Image8 playIcon(HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE);
Animator playTest = Animator(0.1, 1, 5000, true);
void setup() {
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
  playIcon.setSize(180);
  playIcon.setData(home_large_test);
  playTest.reverse = false;
}

//-------------BEFORE LOOP----------------//
uint64_t start = millis();
static bool isDone = false;
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = false;
static float scaling_factor = 0.7f;

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
    
    playTest.update();
    renderBmp8(46,14, playIcon, playTest.progress, 0xffff);
    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT);
    start = millis();
}  



/**************************************************************************/
/*!
   @brief      Easily render a scaled monochrome bitmap to the screen, memory managed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    img  Image8 object containing the byte array and size
    @param    scaling Scaling factor of the bitmap
    @param    color 16-bit color with which the bitmap should be drawn
*/
/**************************************************************************/
void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color){
  Image8 scaled = scale(img, scaling);
  canvas.drawBitmap(x,y, scaled.data.get(), scaled.width, scaled.height, color);
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

