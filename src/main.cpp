#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <images/home_images.h>
#include <UIAssist.h>
#include <HardwareAid.h>

#define SDA 21
#define SCL 22
#define DC 23
#define RST 17
#define SCREENHEIGHT 64
#define SCREENWIDTH 128

using namespace ButtonUtils;

SPIClass spi = SPIClass(VSPI);
Adafruit_ST7735 tft = Adafruit_ST7735(&spi, -1, DC, RST);
GFXcanvas16 canvas = GFXcanvas16(SCREENWIDTH, SCREENHEIGHT);


//--------------------------UI SETUP-----------------------------//
Button button1(26);
Button button2(25);
std::vector<Button*> buttons = {&button1, &button2};

Image8 playTest(HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, home_large_test, 0xffff);
Image8 smallPlayTest(HOME_SMALL_TEST_SIZE, HOME_SMALL_TEST_SIZE, home_small_test, 0xffff);

Image8 largeGallery(HOME_LARGE_GALLERY_SIZE, HOME_LARGE_GALLERY_SIZE, home_large_gallery, 0xffff);
Image8 smallGallery(HOME_SMALL_GALLERY_SIZE, HOME_SMALL_GALLERY_SIZE, home_small_gallery, 0xffff);

Image8 largeSettings(HOME_LARGE_SETTINGS_SIZE, HOME_LARGE_SETTINGS_SIZE, home_large_settings, 0xffff);
Image8 smallSettings(HOME_SMALL_SETTINGS_SIZE, HOME_SMALL_SETTINGS_SIZE, home_small_settings, 0xffff);

AnimatedMonoApp play(smallPlayTest, playTest, 64, 32, true);
AnimatedMonoApp settings(smallSettings, largeSettings, 25, 32, true);
AnimatedMonoApp gallery(smallGallery, largeGallery, 103, 32, true);

std::vector<UIElement*> elements = {&play, &settings, &gallery};
Scene home(elements, play);
UI ui(home, canvas);
//--------------------------UI SETUP-----------------------------//


void setup() {
  setupButtons(buttons);
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
}

//-------------BEFORE LOOP----------------//
uint64_t start = millis();
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = true;
unsigned int frameTime=0;
unsigned int calculationsTime=0;
//-------------SETTINGS----------------//

void framerate(bool render){
  if(render){
    canvas.setCursor(0,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_GREEN);
    canvas.setTextWrap(false);
    canvas.print(frameTime);
  }
}

void computeTime(bool render){
  if(render){
    canvas.setCursor(64,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_RED);
    canvas.setTextWrap(false);
    canvas.print(calculationsTime);
  }
}

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);

void loop() {
    canvas.fillScreen(0x0000); //Fill the background with a black frame

    updateButtons(buttons);  //Update button states for every button
    
    if (button1.clickedOnce && !button2.clickedOnce ) {
      ui.focusDirection(RIGHT);
    }
    if (button2.clickedOnce&& !button1.clickedOnce ) {
      ui.focusDirection(LEFT);
    }

    ui.render();
    
    computeTime(render_frametime);
    framerate(render_frametime);  //Render the framerate in the bottom-left corner on top of everything
    calculationsTime = micros()-start;
    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT); //RENDER THE FRAME
    
  //TEMPORAL VARIABLES AND FUNCTIONS
    rememberButtons(buttons);
    ui.update();
    frameTime = micros()-start;
    start = micros();
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
  tft.setAddrWindow(x, y, w, h);
  tft.writePixels(bitmap, w * h, false);
  tft.endWrite();
}