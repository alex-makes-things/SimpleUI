#include "nicerlandscape.h"
#include <images/home_images.h>
#include <UIAssist.h>
#include <constants.h>
#include <HardwareAid.h>

Button button(26);
std::vector<Button*> buttons = {&button};

MonoImage play(home_large_test, HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, 46, 16);
void setup() {
  button.setup();
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);
  play.InitAnim(0.1, 1, 2500);
  play.anim.setBreathing(true);
  play.anim.setLoop(true);
}

//-------------BEFORE LOOP----------------//
uint64_t start = millis();
//-------------BEFORE LOOP----------------//


//-------------SETTINGS----------------//
bool render_frametime = true;
//-------------SETTINGS----------------//

void framerate(bool render){
  if(render){
    canvas.setCursor(0,50);
    canvas.setTextSize(2);
    canvas.setTextColor(ST7735_GREEN);
    canvas.setTextWrap(false);
    canvas.print(millis()-start);
  }
}

void loop() {
    canvas.fillScreen(0x0000);
    framerate(render_frametime);

    updateButtons(buttons);
    play.render();

    if (button.clickedOnce){
      render_frametime = !render_frametime;
    }

    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT);
    start = millis();
    rememberButtons(buttons);
}  