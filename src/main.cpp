#include "nicerlandscape.h"
#include <images/home_images.h>
#include <UIAssist.h>
#include <constants.h>
#include <HardwareAid.h>

Button button(26);
std::vector<Button*> buttons = {&button};

Image8 playTest(HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, home_large_test, 0xffff);
Image8 smallPlayTest(HOME_SMALL_TEST_SIZE, HOME_SMALL_TEST_SIZE, home_small_test, 0xffff);
MonoImage play(&playTest, 64, 32);
void setup() {
  button.setup();
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
  play.setImg(&smallPlayTest);
  play.centered=true;
  play.InitAnim(1,1.44,100);
  play.anim.stop();
}

//-------------BEFORE LOOP----------------//
uint64_t start = millis();
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = true;
unsigned int frameTime=0;
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

void loop() {
    canvas.fillScreen(0x0000);

    updateButtons(buttons);
    
    play.render();

    if(button.clickedOnce){
      if(play.getImg()==&smallPlayTest && play.anim.getStart()){
        play.overrideScaling = false;
        play.anim.start();
      }
      else if(play.getImg()==&playTest && play.anim.getDone()){
        play.overrideScaling = false;
        play.setImg(&smallPlayTest);
        play.anim.resetAnim();
        play.anim.stop();
      }
    }
    if(play.anim.getDone()&&play.getImg()==&smallPlayTest){
      play.overrideScaling = true;
      play.setImg(&playTest);
      play.setScale(1);
    }
    
    framerate(render_frametime);
    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT);
    rememberButtons(buttons);
    frameTime = millis()-start;
    start = millis();
}  