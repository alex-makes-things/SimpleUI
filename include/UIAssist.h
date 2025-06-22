#pragma once
#include <ImageAssist.h>
#include <constants.h>
#include <functional>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>


#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

using std::string;

//-----------FUNCTION PROTOTYPES----------------//

void fastRender(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color);

//-----------FUNCTION PROTOTYPES----------------//



struct Coordinates{
  int x=0;
  int y=0;
  Coordinates(int posx=0, int posy=0):x(posx), y(posy){}
};


struct Identity{
  unsigned int scene_id=0;
  unsigned int ele_id=0;
  Identity(unsigned int scene=0, unsigned int element=0) : scene_id(scene), ele_id(element){}
};



class Animator{
  private:
  bool isAtStart = true;
  bool isDone = false;   //Signals if the animation is complete or not
  bool looping = false;  //Makes the animation loop if true
  bool reverse = false;  //"Inverts" the final and initial values, (just in the calculations)
  bool breathing = false;
  bool bounce_done = false;
  bool enable=false;
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
    if(reverse){
      //Magnetic attachment to initial value
        if (fabs(progress-final)<=0.005f){
          progress = final;
        }
        isAtStart = (progress==final) ? true : false;
      }else{
        if (fabs(progress-initial)<=0.005f){
          progress = initial;
        }
        isAtStart = (progress==initial) ? true : false;
      }
    if (enable){
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

  void start(){enable = true;currentTime = micros()-elapsed;}
  void stop(){
    update();
    enable = false;
  }
  void switchState(){
    if (!enable){
      currentTime = micros()-elapsed;
    }else{update();}
    enable = !enable;
  }
    

  //Returns the current progress
  float getProgress(){return progress;}

  //Returns the completion state
  bool getDone(){return isDone;}

  bool getStart(){return isAtStart;}

  bool getIsEnabled(){return enable;}
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
    isAtStart = true;
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
    Coordinates position;
    unsigned int width=0, height=0;
    Identity id;
    public:
    Animator anim;
    bool centered = false;
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
    UIElement(unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0){
        width = w;
        height = h;
        position = Coordinates(posx, posy);
    }
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    void setPosX(unsigned int X){position.x = X;}
    void setPosY(unsigned int Y){position.y = Y;}
    void setPos(unsigned int X, unsigned int Y){position.x=X;position.y=Y;}
    void setID(unsigned int i){id.ele_id=i;}
    void setSceneID(unsigned int i){id.scene_id=i;}
    void setIdentity(unsigned int scene, unsigned int own){
      id.scene_id = scene;
      id.ele_id = own;
    }
    Identity getId(){return id;}
    Coordinates getPos(){return position;}
    unsigned int getWidth(){return width;}
    unsigned int getHeight(){return height;}
    virtual void render() = 0;
    void setCenter(bool center){centered = center;}
    Coordinates centerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h){
      unsigned int new_x = floor((float)x_pos-((float)w/2));
      unsigned int new_y = floor((float)y_pos-((float)h/2));
      return Coordinates(new_x, new_y);
    }
};

class MonoImage : public UIElement{
  protected:
    Image8* body;
    float scale_fac=1;
    uint16_t color = 0xffff;
  public:
    bool overrideScaling = false;
    
    MonoImage(Image8 *img, unsigned int posx=0, unsigned int posy=0, unsigned int id=0):UIElement(img->width, img->height, posx, posy){
        body = img;
        setID(id);
    }

    void setScale(float scale){scale_fac = scale;}
    float getScale(){return scale_fac;}
    void setColor(uint16_t hue){color = hue;}
    void setImg(Image8* img){
      body = img;
      width = img->width;
      height = img->height;
      color = img->color;
    }
    Image8* getImg(){
      return body;
    }
    void render() override {
      if (draw){
        anim.update();
        scale_fac = (overrideScaling) ? scale_fac : anim.getProgress();
        Coordinates drawing_pos;
        if (scale_fac != 1){
          std::shared_ptr<Image8> scaled = scale(*body, scale_fac);
          drawing_pos = (centered) ? centerPos(position.x, position.y, scaled->width, scaled ->height) : position;
          canvas.drawBitmap(drawing_pos.x, drawing_pos.y, scaled->data, scaled->width, scaled->height, color);
        }else{
          drawing_pos = (centered) ? centerPos(position.x, position.y, body->width, body->height) : position;
          canvas.drawBitmap(drawing_pos.x, drawing_pos.y, body->data, body->width, body->height, color);
        }

      }
    }
};

struct Scene{
  unsigned int id;
  bool isActive;
  std::unordered_map<unsigned int, UIElement*> elements;

  Scene(std::vector<UIElement*> elementGroup = {}, unsigned int scene_id = 0, bool active = false){
    for(int i = 0; i<elementGroup.size(); i++){
      elements.insert({elementGroup[i]->getId().ele_id,elementGroup[i]});
    }
    id = scene_id;
    isActive = active;
    for(UIElement* elem : elementGroup){
      elem->setSceneID(id);
    }
  }

  int howManyElements(){
    return elements.size();
  }

  void renderScene(){
    if(isActive){
      for(auto elem : elements){
        elem.second->render();
      }
    }
  }
};

struct Focus{
  Identity current;
  Identity previous;
  bool isFirstBoot = true;
  Focus(unsigned int sc=0, unsigned int ele=0){
    current.scene_id = sc;
    current.ele_id = ele;
  }
  void focus(unsigned int sc, unsigned int ele){
    previous.scene_id = current.scene_id;
    previous.ele_id = current.ele_id;
    current.scene_id = sc;
    current.ele_id = ele;
  }
  void update(){
    previous.scene_id = current.scene_id;
    previous.ele_id = current.ele_id;
    isFirstBoot = false;
  }
  bool hasChanged(){
    return (previous.scene_id != current.scene_id || previous.ele_id != current.ele_id );
  }
  bool isFocusing(Identity obj){
    return (current.ele_id == obj.ele_id && current.scene_id == obj.scene_id);
  }
};


namespace UiUtils{

  bool areStill(std::vector<MonoImage*> images){
    int count = 0;
    for (MonoImage* image : images) {
      if(image->anim.getDone() || image->anim.getStart()){
        count++;
      }
    }
      return (count == images.size()) ? true : false;
  }

  bool isPointInElement(Coordinates point, UIElement* element){
    Coordinates element_pos = element->getPos();
    if ((point.x >= element_pos.x && point.x <= element_pos.x + element->getWidth()) && (point.y >= element_pos.y && point.y <= element_pos.y + element->getHeight())){
      return true;
    }
    return false;
  }

  Coordinates centerPos(int x_pos, int y_pos, unsigned int w, unsigned int h){
      int new_x = floor((float)x_pos+((float)w/2));
      int new_y = floor((float)y_pos+((float)h/2));
      return Coordinates(new_x, new_y);
  }

  UIElement* isThereACollision(unsigned int direction, Scene* scene, UIElement* focused, unsigned int max_distance){

    Coordinates center_point = (focused->centered) ? focused->getPos() 
                                                   : UiUtils::centerPos(focused->getPos().x, focused->getPos().y, focused->getWidth(), focused->getHeight());
    Scene temporaryScene = *scene;
    temporaryScene.elements.erase(focused->getId().ele_id);
    Coordinates new_point;
    switch(direction){
      case RIGHT:
        for(int i = 0; i<=max_distance;i++)
        {
          new_point = Coordinates(center_point.x+i, center_point.y);

          for(auto elem : temporaryScene.elements){
            if(isPointInElement(new_point, elem.second)){
              return elem.second;
            }
          }
        }
        break;
      case LEFT:
        for(int i = 0; i<=max_distance;i++)
        {
          new_point = Coordinates(center_point.x-i, center_point.y);

          for(auto elem : temporaryScene.elements){
            if(isPointInElement(new_point, elem.second)){
              return elem.second;
            }
          }
        }
        break;
      case UP:
        for(int i = 0; i<=max_distance;i++)
        {
          new_point = Coordinates(center_point.x, center_point.y+i);

          for(auto elem : temporaryScene.elements){
            if(isPointInElement(new_point, elem.second)){
              return elem.second;
            }
          }
        }
        break;
      case DOWN:
        for(int i = 0; i<=max_distance;i++)
        {
          new_point = Coordinates(center_point.x, center_point.y-i);

          for(auto elem : temporaryScene.elements){
            if(isPointInElement(new_point, elem.second)){
              return elem.second;
            }
          }
        }
        break;
    }
  return nullptr;
  }

}

class UI{
  public:
  Focus focus;
  std::unordered_map<unsigned int, Scene*> scenes;
  UIElement* focusedElement = nullptr;
  Scene* focusedScene = nullptr;
  unsigned int max_focusing_distance = 64;

  UI(Identity first_focus, Scene* first_scene){
    addScene(first_scene);
    focusScene(first_scene);
    focusedElement = first_scene->elements[first_focus.ele_id];
    focus = Focus(first_scene->id, focusedElement->getId().ele_id);
  }

  void addScene(Scene* scene){
    scenes.insert({scene->id, scene});
  }
  void focusScene(Scene* scene){
    focusedScene = scene;
  }
  void render(){
    focusedScene->renderScene();
  }
  void focusDirection(unsigned int direction){
    UIElement* next_element = UiUtils::isThereACollision(direction, focusedScene, focusedElement, max_focusing_distance);
    if(next_element){
        focus.focus(next_element->getId().scene_id, next_element->getId().ele_id);
        focusedElement = next_element;
    }
  }
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

