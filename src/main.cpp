#include "nicerlandscape.h"
#include <images/home_images.h>
#include <UIAssist.h>
#include <constants.h>
#include <HardwareAid.h>

Button button1(26);
Button button2(25);
std::vector<Button*> buttons = {&button1, &button2};

Image8 playTest(HOME_LARGE_TEST_SIZE, HOME_LARGE_TEST_SIZE, home_large_test, 0xffff);
Image8 smallPlayTest(HOME_SMALL_TEST_SIZE, HOME_SMALL_TEST_SIZE, home_small_test, 0xffff);

Image8 largeGallery(HOME_LARGE_GALLERY_SIZE, HOME_LARGE_GALLERY_SIZE, home_large_gallery, 0xffff);
Image8 smallGallery(HOME_SMALL_GALLERY_SIZE, HOME_SMALL_GALLERY_SIZE, home_small_gallery, 0xffff);

Image8 largeSettings(HOME_LARGE_SETTINGS_SIZE, HOME_LARGE_SETTINGS_SIZE, home_large_settings, 0xffff);
Image8 smallSettings(HOME_SMALL_SETTINGS_SIZE, HOME_SMALL_SETTINGS_SIZE, home_small_settings, 0xffff);

MonoImage play(&playTest, 64, 32, 1);
MonoImage settings(&largeSettings, 25, 32, 0);
MonoImage gallery(&largeGallery, 103, 32, 2);

std::vector<MonoImage*> apps = {&play, &settings, &gallery};
std::vector<UIElement*> elements = {&play, &settings, &gallery};
Scene home(elements, 0, true);
Identity first_focus(0,1);
UI ui(first_focus, &home);

Focus focus(0,1);
void setup() {
  setupButtons(buttons);
  Serial.begin(115200);
  spi.begin(SCL, -1, SDA, -1);
  tft.initR(INITR_GREENTAB);
  tft.setSPISpeed(78000000); //Absolute fastest speed tested, errors at 80000000
  tft.fillScreen(ST7735_BLACK);

  play.setImg(&smallPlayTest);
  settings.setImg(&smallSettings);
  gallery.setImg(&smallGallery);

  play.centered=true;
  settings.centered=true;
  gallery.centered=true;

  play.InitAnim(1,1.44,75);
  settings.InitAnim(1,1.44,75);
  gallery.InitAnim(1,1.44,75);
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

void handleAppSelectionAnimation(MonoImage* app, Image8* unfocused, Image8* focused, bool isPrimary = false){
  if(ui.focus.hasChanged() || (ui.focus.isFirstBoot && isPrimary)){
      if(ui.focus.isFocusing(app->getId())){
        if(app->getImg()==unfocused && app->anim.getStart())
        {
          app->overrideScaling = false;
          app->anim.start();
        }
      } else {
        if(app->getImg()==focused && app->anim.getDone())
        {
          app->overrideScaling = false;
          app->setImg(unfocused);
          app->anim.resetAnim();
          app->anim.invert();
        }
      }
    }
    
    if(app->anim.getDone()&&app->getImg()==unfocused){
      if(ui.focus.isFocusing(app->getId())){
        app->overrideScaling = true;
        app->setImg(focused);
        app->setScale(1);
      }else{
        app->overrideScaling = false;
        app->anim.invert();
        app->anim.resetAnim();
        app->anim.stop();
      }
      
    }
}




void loop() {
    canvas.fillScreen(0x0000); //Fill the background with a black frame

    updateButtons(buttons);  //Update button states for every button
    //home.renderScene();
    ui.render();

    /*
    if (button1.clickedOnce && !button2.clickedOnce && UiUtils::areStill(apps)) {
      if(focus.current.ele_id == 2)
      {focus.focus(0, 0);}
      else
      {focus.focus(0,focus.current.ele_id+1);}
    }
    if (button2.clickedOnce&& !button1.clickedOnce && UiUtils::areStill(apps)) {
      if(focus.current.ele_id == 0)
      {focus.focus(0, 2);}
      else
      {focus.focus(0,focus.current.ele_id-1);}
    }*/

    if (button1.clickedOnce && !button2.clickedOnce && UiUtils::areStill(apps)) {
      ui.focusDirection(RIGHT);
    }
    if (button2.clickedOnce&& !button1.clickedOnce && UiUtils::areStill(apps)) {
      ui.focusDirection(LEFT);
    }

    handleAppSelectionAnimation(&play, &smallPlayTest, &playTest, true);
    handleAppSelectionAnimation(&settings, &smallSettings, &largeSettings);
    handleAppSelectionAnimation(&gallery, &smallGallery, &largeGallery);


    framerate(render_frametime);  //Render the framerate in the bottom-left corner on top of everything
    fastRender(0,0,canvas.getBuffer(),SCREENWIDTH,SCREENHEIGHT); //RENDER THE FRAME
    
  //TEMPORAL VARIABLES AND FUNCTIONS
    rememberButtons(buttons);
    focus.update();
    frameTime = millis()-start;
    start = millis();
}  