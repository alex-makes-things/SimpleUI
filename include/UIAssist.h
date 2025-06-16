#include <ImageAssist.h>
#include <constants.h>

//-----------FUNCTION PROTOTYPES----------------//

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color);

//-----------FUNCTION PROTOTYPES----------------//

const float EPSILON = 0.01; //The epsilon is needed because of floating point precision, and we need a tolerance margin, so sometimes "==" wouldn't work

class Animator{
  private:
  bool isDone = false;   //Signals if the animation is complete or not
  bool looping = false;  //Makes the animation loop if true
  bool reverse = false;  //"Inverts" the final and initial values, (just in the calculations)
  bool breathing = false;
  bool bounce_done = false;
  unsigned int duration; //How long the animation takes to go from initial to final and viceversa
  float initial;
  float final;
  float progress;  //The progress is ultimately the output of the structure, which lies clamped in between initial and final
  uint64_t currentTime = millis();   //This is needed for the temporal aspect of the interpolation
  void invert(){ //Function that can be called at runtime which inverts the direction of the interpolation
    //WORK IN PROGRESS
    reverse = !reverse;
  }
  public:
  Animator(float i=0, float f=0, unsigned int d=0){  //Basic constructor
    duration = d;
    initial = i;
    final = f;
    progress = initial;
  }

  void update(){
    //Breathing feature snippet
    if(breathing&&isDone){
      if(looping){
        invert();
        bounce_done = false;
      }else if(progress==final&&(!bounce_done)){
        invert();
        isDone=false;
        currentTime = millis();
        bounce_done = true;
      }else if (reverse&&(progress==initial)&&(!bounce_done)){
        invert();
        isDone=false;
        currentTime = millis();
        bounce_done = true;
      }
    }

    //Looping feature
    //Has to be before the actual update, otherwise you never know when the animation reached the target value.
    if(isDone && looping){  //Independent check that loops the animation by resetting the done status, progress, and internal timing.
      isDone = false;
      progress = (reverse) ? final : initial;
      currentTime = millis();
    }

    //Actual progress calculation
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
  }
  void setDuration(unsigned int d){duration = d;}
  void setInitial(unsigned int i){initial = i;}
  void setFinal(unsigned int f){final = f;}
  void setLoop(bool loop){looping = loop;}
  void setBreathing(bool breathe){breathing = breathe;}
  void setReverse(bool rev){
    reverse = rev;
    progress = (progress==initial&&reverse) ? final : progress;
  }
  float getProgress(){return progress;}
  bool getDone(){return isDone;}
  void defaultModifiers(){
    looping = false;
    reverse = false;
    breathing = false;
    bounce_done = false;
  }
};


class UIElement{
    protected:
    unsigned int width=0, height=0, x = 0, y = 0;

    public:
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
    UIElement(unsigned int w, unsigned int h, unsigned int posx, unsigned int posy){
        width = w;
        height = h;
        x = posx;
        y = posy;
    }
    void setPosX(unsigned int X){x = X;}
    void setPosY(unsigned int Y){y = Y;}
    void setPos(unsigned int X, unsigned int Y){x=X;y=Y;}
};

class MonoImage : UIElement{
  protected:
    Image8 body;
    float scale_fac=1;
    uint16_t color = 0xffff;
  public:
    Animator anim;
    MonoImage(const uint8_t* input, unsigned int w, unsigned int h, unsigned int posx, unsigned int posy):UIElement(w, h, posx, posy){
        body = Image8(w,h,input);
    }
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    void setScale(float scale){scale_fac = scale;}
    virtual void render(){
      anim.update();
      scale_fac = anim.getProgress();
      renderBmp8(x, y, body, scale_fac, color);
    }
};

//class MonoApp : MonoImage{};

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
    Image8 scaled = scale(img, scaling);
    canvas.drawBitmap(x,y, scaled.data.get(), scaled.width, scaled.height, color);
  }else{
    canvas.drawBitmap(x,y, img.data.get(), img.width, img.height, color);
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

