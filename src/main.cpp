#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <images/home_images.h>
#include <SimpleUI.h>
#include <HardwareAid.h>
#include <Animation.h>
#include "images/splash_screen.h"

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


AnimatedApp play    (&smallPlayTest, &playTest,     {64, 32},  true, 80U, Interpolation::Sinusoidal, Constraint::Center);
AnimatedApp settings(&smallSettings, &largeSettings,{25, 32},  true, 80U, Interpolation::Sinusoidal, Constraint::Center);
AnimatedApp gallery (&smallGallery , &largeGallery, {103, 32}, true, 80U, Interpolation::Sinusoidal, Constraint::Center);
Scene home({&play, &settings, &gallery}, nullptr);

Checkbox check1(Outline(2, 2, 5, 0xFFFF), {0 ,5}, 16, 16, 0xFFFF);
Checkbox check2(Outline(2, 2, 5, 0xFFFF), {20,5}, 16, 16, 0xFFFF);
Checkbox check3(Outline(2, 2, 5, 0xFFFF), {40,5}, 16, 16, 0xFFFF);
Scene test({&check1, &check2, &check3}, &check1);
UI ui(&home, &canvas);

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
        UIElement *obj = ui.getFocused();
        Serial.printf("ID: %s\n", ui.focus.focusedElementID.c_str());
        Serial.printf("Position: (%d, %d)\n", obj->getPos().x, obj->getPos().y);
        switch(obj->anim.getState()){
          case AnimState::Start: Serial.println("AnimState: Start"); break;
          case AnimState::Running: Serial.println("AnimState: Running"); break;
          case AnimState::Finished: Serial.println("AnimState: Finished"); break;
        }
        Serial.printf("Direction: %s\n", obj->anim.getDirection() ? "Forward" : "Reverse");
        Serial.printf("Draw: %s\n", obj->draw ? "true" : "false");
        if(obj->getType() == ElementType::UIElement)
          Serial.println("Type: UIElement");
        else if (obj->getType() == ElementType::UIImage)
          Serial.println("Type: UIImage");
        else if (obj->getType() == ElementType::Checkbox)
          Serial.println("Type: Checkbox");
        else 
          Serial.println("Type: AnimatedApp");
        Serial.printf("Focusable: %s\n", obj->focusable ? "true" : "false");
        Serial.printf("Custom outline: %s\n", obj->custom_focus_outline ? "true" : "false");
        Serial.printf("Constraint: %s\n", UiUtils::constraintToString(obj->scale_constraint).c_str());
        Serial.println("----Scene----");
        Serial.printf("Max focusing distance: %d\n", ui.getActiveScene()->settings.focus.max_distance);
        Serial.printf("Focusing algorithm: %s\n", ui.getActiveScene()->settings.focus.algorithm == FocusingAlgorithm::Linear ? "Linear" : "Cone");
        Serial.printf("Focusing accuracy: %s\n", ui.getActiveScene()->settings.focus.accuracy == Quality::Low ? "Low" : ui.getActiveScene()->settings.focus.accuracy == Quality::Medium ? "Medium" : "High");
        Serial.printf("Script on top: %s\n", ui.getActiveScene()->settings.scriptOnTop ? "true" : "false");
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


Animation myAnimation(118.0f, 0.0f, 1000U, Interpolation::Sinusoidal);
uint32_t calcStart = micros(), lastFrame = micros(), deltaTime = 0;



//-------------SETTINGS----------------//
bool render_frametime = true;
unsigned int fpsTarget = FPS90;
unsigned int calculationsTime=0;
uint16_t debugColor = hex("#ff8e00");

auto loadTest = [&](){
  ui.FocusScene(&test);
  myAnimation.Reset();
  myAnimation.Start();
};

auto testSceneScript = [&](){
  canvas.fillRect(int(myAnimation.getProgress()), 0, 10, 10, ST7735_ORANGE);
  myAnimation.Update();
    if(myAnimation == AnimState::Finished)
      myAnimation.Flip();
};

//-------------SETTINGS----------------//
inline __attribute__((always_inline))
void blit()
{
  tft.setAddrWindow(0, 0, 128, 64);
  tft.writePixels(canvas.getBuffer(), 8192, false);
}
void framerate(bool render){
  if(render){
    canvas.setCursor(0,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_GREEN);
    canvas.setTextWrap(false);
    canvas.print(1000000/deltaTime);
    canvas.setCursor(canvas.getCursorX(),57);
    canvas.setTextSize(1);
    canvas.print("FPS");
  }
}
void computeTime(bool render){
  if(render){
    canvas.setCursor(60,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_RED);
    canvas.setTextWrap(false);
    canvas.print(calculationsTime);
  }
}
void initLCD(){
  pinMode(BACKLIGHT, OUTPUT);
  analogWrite(BACKLIGHT, 0);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  //tft.setSPISpeed(75000000); //Absolute fastest speed tested, errors at 80000000
  canvas.fillScreen(ST7735_BLACK);
  blit();
  delay(10);
  Point pos = UIElement::centerToCornerPos(64, 32, 28, 30);
  tft.drawBitmap(pos.x, pos.y, splash_logo, 28, 30, 0xffff);

  Serial.begin(115200);
  analogWrite(BACKLIGHT, 50);
}




void setup() {

  initLCD();

  setupButtons(buttons);

  xTaskCreatePinnedToCore(handleComms, "Comms", 2000, NULL, 1, &serialComms, 0);

  home.settings.focus.outline = Outline(2, 2, 3);
  test.settings.focus.outline = Outline(1, 1, 7, hex("#6b6b6b"));
  ui.AddScene(&test);
  play.bind(loadTest);
  test.addParents({&home});
  test.Script(testSceneScript, true);
  myAnimation.setLoop(true);

  delay(1000);
}


void loop() {
  deltaTime = micros() - lastFrame;

  updateButtons(buttons);  //Update button states for every button
  
  if (button1.clickedOnce && !button2.clickedOnce ) {
    ui.FocusDirection(Direction::Right);
  }
  if (button2.clickedOnce&& !button1.clickedOnce ) {
    
    ui.FocusDirection(Direction::Left);
  }
  
  if(button3.clickedOnce){
    ui.Click();
  }

  if (deltaTime >= fpsTarget){
    lastFrame = micros();
    
    canvas.fillScreen(0x0000); //Fill the background with a black frame
    

    calcStart = micros();
    ui.Render();
    calculationsTime = micros() - calcStart;

    computeTime(render_frametime);
    framerate(render_frametime);  //Render the framerate in the bottom-left corner on top of everything

    blit(); //RENDER THE FRAME

    //TEMPORAL VARIABLES AND FUNCTIONS
    
  }
  rememberButtons(buttons);
}