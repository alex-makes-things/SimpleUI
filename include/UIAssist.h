#pragma once
#include <ImageAssist.h>
#include <constants.h>
#include <functional>


//-----------FUNCTION PROTOTYPES----------------//

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color);

//-----------FUNCTION PROTOTYPES----------------//

class Animator{
  private:
  bool isDone = false;   //Signals if the animation is complete or not
  bool looping = false;  //Makes the animation loop if true
  bool reverse = false;  //"Inverts" the final and initial values, (just in the calculations)
  bool breathing = false;
  bool bounce_done = false;
  unsigned int duration; //How long the animation takes to go from initial to final and viceversa, input as ms but converted to us
  float initial;
  float final;
  float progress;  //The progress is ultimately the output of the structure, which lies clamped in between initial and final
  uint64_t currentTime = micros();   //This is needed for the temporal aspect of the interpolation
  uint64_t now = micros();
  uint64_t elapsed = 0;
  
  public:
  Animator(float i=0, float f=0, unsigned int d=0){  //Basic constructor
    duration = d*1000;
    initial = i;
    final = f;
    progress = initial;
  }

  //Updates the animation logic and attributes
  void update(){
    now = micros();
    elapsed = clamp(now-currentTime, 0, duration);
    if(isDone){
      if(breathing){
        if(looping){
          invert();
          bounce_done = false;
        } else if (!bounce_done){
          invert();
          isDone=false;
          bounce_done = true;
        }
      }
      if(looping){
        resetAnim();
      }
    }

    if(!isDone){
      if (elapsed<duration){
        float t = (reverse) ? clamp(fabs(1-mapF(elapsed, 0, duration, 0, 1)),0,1) : clamp(mapF(elapsed, 0, duration, 0, 1),0,1);
        progress = lerpF(initial, final, t);
        } else {
          isDone = true;
          progress = (reverse) ? initial : final;
        }
    }
    
  }

  //Set the duration of the animation to the passed unsigned integer
  void setDuration(unsigned int d){duration = d;}

  //Set the initial value to the passed unsigned integer
  void setInitial(unsigned int i){initial = i;}

  //Set the final value to the passed unsigned integer
  void setFinal(unsigned int f){final = f;}

  //Set the looping setting to the passed bool
  void setLoop(bool loop){looping = loop;}

  //Set the breathing setting to the passed bool
  void setBreathing(bool breathe){breathing = breathe;}

  //Set the reverse setting to the passed bool
  void setReverse(bool rev){
    reverse = rev;
    progress = (progress==initial&&reverse) ? final : progress;
  }

  //Returns the current progress
  float getProgress(){return progress;}

  //Returns the completion state
  bool getDone(){return isDone;}

  //Resets the animation's settings to the defaults (false)
  void defaultModifiers(){
    looping = false;
    reverse = false;
    breathing = false;
    bounce_done = false;
  }

  //Makes the animation restart
  void resetAnim(){
    progress = (reverse) ? final : initial;
    currentTime = micros();
    isDone = false;
  }

  //Function that can be called at any time which inverts the direction of the animation
  void invert(){ 
    reverse = !reverse;
    elapsed = duration-elapsed;
    currentTime = now-elapsed;
    now = currentTime;
  }
};


class UIElement{
    protected:
    unsigned int width=0, height=0, x = 0, y = 0;
    unsigned int scene_id = 0;
    unsigned int own_id = 0;
    public:
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
    UIElement(unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0){
        width = w;
        height = h;
        x = posx;
        y = posy;
    }
    void setPosX(unsigned int X){x = X;}
    void setPosY(unsigned int Y){y = Y;}
    void setPos(unsigned int X, unsigned int Y){x=X;y=Y;}
    virtual void render() = 0;
};

class MonoImage : public UIElement{
  protected:
    Image8 body;
    float scale_fac=1;
    uint16_t color = 0xffff;
  public:
    Animator anim;
    MonoImage(const uint8_t* input = nullptr, unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0):UIElement(w, h, posx, posy){
        body = Image8(w,h,input);
    }
    MonoImage(const uint8_t* input, unsigned int w, unsigned int h){
        body = Image8(w,h,input);
        width = w;
        height = h;
    }
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    void setScale(float scale){scale_fac = scale;}
    void setColor(uint16_t hue){color = hue;}
    void render() override {
      if (draw){
        anim.update();
        scale_fac = anim.getProgress();
        renderBmp8(x, y, body, scale_fac, color);
      }
    }
};

//DO NOT USE
class MonoApp : public MonoImage{
  std::function<void()> onHover = 0;
  std::function<void()> onClick = 0;
  bool isHovered = false;
  bool isClicked = false;
  MonoApp(const uint8_t* input = nullptr, unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0):MonoImage(input, w, h, posx, posy){}
  void setOnHover(std::function<void()> hover){
    onHover = hover;
  }
  void setOnClick(std::function<void()> click){
    onClick = click;
  }
  void render() override {
    if (draw){
      if (isHovered){
        onHover();
      }
      if (isClicked){
        onClick();
      }
    }
  }
};

struct Focus{
  unsigned int scene_focus = 0;
  unsigned int element_focus = 0;
};

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
  if(scaling != 1){
    //Image8 scaled = scale(img, scaling);
    std::shared_ptr<Image8> scaled = scale(img, scaling);
    canvas.drawBitmap(x,y, scaled->data, scaled->width, scaled->height, color);
  }else{
    canvas.drawBitmap(x,y, img.data, img.width, img.height, color);
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
void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  tft.startWrite();
  tft.setAddrWindow(x,y,w,h);
  tft.writePixels(bitmap, w*h, false);
  tft.endWrite();
}

