#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <images/home_images.h>
#include <images/nicerlandscape.h>
#include <SimpleUI.h>
#include <HardwareAid.h>
#include <Animation.h>

#define SDA 21
#define SCL 22
#define DC 23
#define RST 17
#define SCREENHEIGHT 64
#define SCREENWIDTH 128
#define BACKLIGHT 4

using namespace ButtonUtils;
using namespace SimpleUI;

SPIClass spi(VSPI);
Adafruit_ST7735 tft(&spi, -1, DC, RST);
GFXcanvas16 canvas(SCREENWIDTH, SCREENHEIGHT);


//--------------------------UI SETUP-----------------------------//
Button button1(26);
Button button2(25);
Button button3(27);
std::vector<Button*> buttons = {&button1, &button2, &button3};

Texture playTest(HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, home_large_test);
Texture smallPlayTest(HOME_SMALL_TEST_SIZE, HOME_SMALL_TEST_SIZE, home_small_test);

Texture largeGallery(HOME_LARGE_GALLERY_SIZE, HOME_LARGE_GALLERY_SIZE, home_large_gallery);
Texture smallGallery(HOME_SMALL_GALLERY_SIZE, HOME_SMALL_GALLERY_SIZE, home_small_gallery);

Texture largeSettings(HOME_LARGE_SETTINGS_SIZE, HOME_LARGE_SETTINGS_SIZE, home_large_settings);
Texture smallSettings(HOME_SMALL_SETTINGS_SIZE, HOME_SMALL_SETTINGS_SIZE, home_small_settings);


AnimatedApp play    (smallPlayTest, playTest,     {64, 32},  80U, true);
AnimatedApp settings(smallSettings, largeSettings,{25, 32},  80U, true);
AnimatedApp gallery (smallGallery , largeGallery, {103, 32}, 80U, true);

Checkbox check1(Outline(2, 2, 5, 0xFFFF), {0 ,5},16, 16, 0xFFFF);
Checkbox check2(Outline(2, 2, 5, 0xFFFF), {20,5},16, 16, 0xFFFF);
Checkbox check3(Outline(2, 2, 5, 0xFFFF), {40,5},16, 16, 0xFFFF);

std::vector<UIElement*> elements = {&play, &settings, &gallery};
std::vector<UIElement*> testing = {&check1, &check2, &check3};
std::vector<UIElement*> empty = {};
Scene home(elements, &play);
//Scene test(testing, check1);
Scene test(empty, nullptr);
UI ui(home, canvas);
//--------------------------UI SETUP-----------------------------//

TaskHandle_t serialComms;
void handleComms( void *pvParameters){
  Serial.setTimeout(250);

  while(true){
    if (Serial.available()){
      String input = Serial.readString();
      input.trim();
      if (input == "freemem")
      {
        Serial.printf("Used heap: %fkb / %fkb\n", (static_cast<float>(ESP.getHeapSize()) / 1000.0) - static_cast<float>(ESP.getFreeHeap()) / 1000.0, static_cast<float>(ESP.getHeapSize()) / 1000.0);
      }
      else if(input == "version"){
        Serial.println(ESP.getSdkVersion());
      }
      else if (input == "reset")
      {
        Serial.println("Rebooting...");
        ESP.restart();
      }
      else if (input == "uptime")
      {
        Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
      }
      else if (input == "freq")
      {
        Serial.printf("Frequency: %dMHz\n", ESP.getCpuFreqMHz());
      }
      else if (input == "debugui")
      {
        UIElement *obj = ui.focus.activeScene->elements.at(ui.focus.focusedElementID);
        Serial.printf("ID: %s\n", ui.focus.focusedElementID.c_str());
        //Serial.printf("isAtStart: %s\n", obj->anim.getStart() ? "true" : "false");
        //Serial.printf("isDone: %s\n", obj->anim.getDone() ? "true" : "false");
        //Serial.printf("isEnabled: %s\n", obj->anim.getIsEnabled() ? "true" : "false");
        Serial.printf("Reverse: %s\n", obj->anim.getDirection() ? "true" : "false");
        Serial.printf("Draw: %s\n", obj->draw ? "true" : "false");
        if(obj->type == ElementType::UIElement)
          Serial.println("Type: UIElement");
        else if (obj->type == ElementType::UIImage)
          Serial.println("Type: UIImage");
        else if (obj->type == ElementType::Checkbox)
          Serial.println("Type: Checkbox");
        else 
          Serial.println("Type: AnimatedApp");
      }
      else if (input == "perfstats")
      {
        #if PERFORMANCE_PROFILING
          ui.printPerfStats();
        #else
          Serial.println("Performance profiling is turned off!");
        #endif
      }
      else if (input == "back")
      {
        ui.Back();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

Animation myAnimation(118.0f, 0.0f, 2500U, Interpolation::Sinusoidal);

void loadTest(){
  ui.FocusScene(&test);
  myAnimation.Reset();
  myAnimation.Start();
}

void setup() {
  setupButtons(buttons);
  pinMode(BACKLIGHT, OUTPUT);
  analogWrite(BACKLIGHT, 50);
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);

  xTaskCreatePinnedToCore(handleComms, "Comms", 2000, NULL, 1, &serialComms, 0);

  play.focus_outline.border_distance = 2;
  settings.focus_outline.border_distance = 2;
  gallery.focus_outline.border_distance = 2;

  play.focus_outline.radius = 3;
  settings.focus_outline.radius = 3;
  gallery.focus_outline.radius = 3;

  play.focus_outline.thickness = 2;
  settings.focus_outline.thickness = 2;
  gallery.focus_outline.thickness = 2;

  check1.focus_outline.color = hex("#6b6b6b");
  check2.focus_outline.color = hex("#6b6b6b");
  check3.focus_outline.color = hex("#6b6b6b");

  check1.focus_outline.border_distance = 1;
  check2.focus_outline.border_distance = 1;
  check3.focus_outline.border_distance = 1;

  check1.focus_outline.radius = 7;
  check2.focus_outline.radius = 7;
  check3.focus_outline.radius = 7;
  ui.AddScene(test);
  play.bind(std::move(loadTest));
  test.addParents({&home});
}

//-------------BEFORE LOOP----------------//
uint32_t start = micros(), calcStart = micros();
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = true;
unsigned int frameTime=0;
unsigned int calculationsTime=0;
uint16_t debugColor = hex("#ff8e00");

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
    canvas.setCursor(54,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_RED);
    canvas.setTextWrap(false);
    canvas.print(calculationsTime);
  }
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
inline __attribute__((always_inline))
void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  tft.setAddrWindow(x, y, w, h);
  tft.writePixels(bitmap, 8192, false);
}


void loop() {
    canvas.fillScreen(0x0000); //Fill the background with a black frame

    updateButtons(buttons);  //Update button states for every button
    
    if (button1.clickedOnce && !button2.clickedOnce ) {
      ui.FocusDirection(Direction::Right, FocusingAlgorithm::Cone);
    }
    if (button2.clickedOnce&& !button1.clickedOnce ) {
      
      ui.FocusDirection(Direction::Left, FocusingAlgorithm::Cone);
    }

    if(button3.clickedOnce){
      ui.Click();
    }
    
    
    
    if (ui.getActiveScene() == &test){
      canvas.fillRect(int(myAnimation.getProgress()), 0, 10, 10, 0xffff);
      myAnimation.Update();
    }
    calcStart = micros();
    ui.Render();
    calculationsTime = micros() - calcStart;

    computeTime(render_frametime);
    framerate(render_frametime);  //Render the framerate in the bottom-left corner on top of everything

    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT); //RENDER THE FRAME

    
  //TEMPORAL VARIABLES AND FUNCTIONS
    rememberButtons(buttons);
    frameTime = micros()-start;
    start = micros();
}