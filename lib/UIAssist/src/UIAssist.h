#pragma once
#include <ImageAssist.h>
#include <vector>
#include <unordered_map>
#include <UUIDbuddy.h>
#include <Adafruit_GFX.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

using std::string;

//-----------FUNCTION PROTOTYPES----------------//

void renderBmp8(int x, int y, Image8 img, float scaling, uint16_t color, GFXcanvas16 *canvas);

//-----------FUNCTION PROTOTYPES----------------//

class UI;
class UIElement;
class MonoImage;
class AnimatedMonoApp;
class Animator;

struct Coordinates{
  int x=0;
  int y=0;
  Coordinates(int posx=0, int posy=0):x(posx), y(posy){}
};

struct Focus{
  std::string current;
  std::string previous;
  bool isFirstBoot = true;
  Focus(std::string ele="");
  /*!
    @brief Focus an object by its id
    @param sc The scene id to focus
    @param ele The element id to focus
  */
  void focus(std::string ele);
  //This function needs to be called at the end of void loop(), which updates the previous focus id to the current one
  void update();
  //!@return A boolean that when true, means that the focus has changed in the last cycle
  bool hasChanged();
  /*!
    @return A boolean that when true, means that the passed object's identity is currently focused
    @param obj The identity to check if focused
  */
  bool isFocusing(std::string obj);
  bool isFocusing(UIElement* obj);
};

class Animator{
  public:
  /*!
    @brief Animation constructor
    @param i Initial value
    @param f Final value
    @param d The duration of the animation from initial to final in milliseconds
  */
  Animator(float i=0, float f=0, unsigned int d=0);

  //Updates the animation logic and attributes
  void update();
  //Set the duration of the animation to the passed unsigned integer
  void setDuration(unsigned int d){duration = d;}
  //Set the initial value to the passed unsigned integer
  void setInitial(float i){
    initial = i;
    progress = (progress < initial) ? initial : progress;}
  //Set the final value to the passed unsigned integer
  void setFinal(float f){
    final = f;
    progress = (progress > final) ? final : progress;
  }
  //Set the looping setting to the passed bool
  void setLoop(bool loop){looping = loop;}
  //Set the breathing setting to the passed bool
  void setBreathing(bool breathe){breathing = breathe;}
  //Set the reverse setting to the passed bool
  void setReverse(bool rev);
  //Starts the animation
  void start();
  //Pauses the animation
  void stop();
  //Switches the enable key, if true it becomes false and viceversa
  void switchState();
  //!@return A float representing the current progress in the animation
  float getProgress(){return progress;}
  //!@return True if the animation is at the ending point
  bool getDone(){return isDone;}
  //!@return True if the animation is at the starting point
  bool getStart(){return isAtStart;}
  //!@return True if the animation is not stopped
  bool getIsEnabled(){return enable;}
  //!@return True if the animation is going backwards
  bool getDirection(){return reverse;}
  //Resets the animation's settings to the defaults (all false)
  void defaultModifiers();
  //Makes the animation restart, works even with reverse
  void resetAnim();
  //Function that can be called at any time which inverts the direction of the animation
  void invert();

  private:
    bool isAtStart = true; //If true, the animation is in its starting point
    bool isDone = false;   //Signals if the animation is complete or not
    bool looping = false;  //Makes the animation loop if true
    bool reverse = false;  //"Inverts" the final and initial values, (just in the calculations)
    bool breathing = false;//Gives a breathing effect to the animation, so it animates back to its starting point after reaching its target
    bool bounce_done = false;
    bool enable=false;     //If false, the animation is paused
    unsigned int duration; //How long the animation takes to go from initial to final and viceversa, input as ms but converted to us
    float initial;
    float final;    
    float progress;  //The progress is ultimately the output of the structure, which lies clamped in between initial and final
    uint64_t currentTime = micros();   //This is needed for the temporal aspect of the interpolation
    uint64_t now = micros();  //This is used for synchronicity so that every cycle uses the same time reference
    uint64_t elapsed = 0;  //The amount of time that has passed while the animation was running
};

class UIElement{
  public:
    Animator anim;
    bool centered = false; //If true, the element will be drawn with its center being the coordinates passed to the constructor
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
    UIElement(unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0);
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    void setPosX(unsigned int X){m_position.x = X;}
    void setPosY(unsigned int Y){m_position.y = Y;}
    void setPos(unsigned int X, unsigned int Y){m_position.x=X;m_position.y=Y;}
    /*!
      @brief Set the focus listener, this is used in different ways by each UIElement child
      @param listener A pointer to the Focus object that wants to be listened to
    */
    void setFocusListener(Focus* listener){m_parent_focus = listener;}
    void setUiListener(UI* listener){m_parent_ui = listener;}
    //!@return An Identity type containing the element's scene and own ID
    std::string getId() { return m_UUID; }
    Coordinates getPos(){return m_position;}
    unsigned int getWidth(){return m_width;}
    unsigned int getHeight(){return m_height;}
    virtual void render() = 0;
    bool isAnimating(){
      return !(anim.getDone() || anim.getStart());
    }
    bool isFocused();
    void setCenter(bool center){centered = center;}
    Coordinates centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h);

  protected:
    Coordinates m_position;
    unsigned int m_width=0, m_height=0;
    const std::string m_UUID = RandomBuddy::generateUUID();
    UI* m_parent_ui;
    Focus* m_parent_focus; //This is the focus oject that the element will listen to for its own updates
};

class MonoImage : public UIElement{
  public:
    bool overrideScaling = false;
    MonoImage(Image8& img, unsigned int posx=0, unsigned int posy=0):UIElement(img.width, img.height, posx, posy){
        m_body = &img;
    }
    void setScale(float scale){m_scale_fac = scale;}
    float getScale(){return m_scale_fac;}
    void setColor(uint16_t hue){m_color = hue;}
    void setImg(Image8* img);
    Image8* getImg(){return m_body;}
    void render() override;

  protected:
    Image8* m_body;
    float m_scale_fac=1;
    uint16_t m_color = 0xffff;
};

class AnimatedMonoApp : public UIElement{
  public:
    AnimatedMonoApp(Image8& unfocused, Image8& focused, int posx, int posy, bool isCentered):UIElement(unfocused.width, unfocused.height, posx, posy){
      centered = isCentered;
      m_unselected = &unfocused;
      m_selected = &focused;
      m_showing = m_unselected;
      m_ratio = static_cast<float>(focused.width) / static_cast<float>(unfocused.width);
      anim = Animator(1, m_ratio, 60);
    }
    
    void render() override;
    Image8* getActive(){return m_showing;}
    void setColor(uint16_t hue){m_color = hue;}
  protected:
    void handleAppSelectionAnimation();
    uint16_t m_color = 0xffff;
    Image8* m_unselected = nullptr; //This points to the image that is displayed when the element is unfocused
    Image8* m_selected = nullptr;  //This points to the image that is displayed when the element is focused
    Image8* m_showing = nullptr;  //This points to the image that is currently being displayed
    float m_ratio;
  };

struct Scene{
  std::string name;
  std::string primaryElementID;
  std::unordered_map<std::string, UIElement*> elements;
  Scene(std::vector<UIElement*> elementGroup, UIElement& first_focus);
  void renderScene();
  UIElement* getElementByUUID(std::string UUID);
};

namespace UiUtils{
  bool areStill(std::vector<MonoImage*> images);
  bool isPointInElement(Coordinates point, UIElement* element);
  Coordinates centerPos(int x_pos, int y_pos, unsigned int w, unsigned int h);
  UIElement* isThereACollision(unsigned int direction, Scene* scene, UIElement* focused, unsigned int max_distance);
}

class UI{
  public:
  Focus focus;
  std::vector<Scene*> scenes;
  std::string focusedElementID;
  Scene* focusedScene = nullptr;
  unsigned int max_focusing_distance = 64;
  bool focus_outline = false;

  UI(Scene& first_scene, GFXcanvas16& framebuffer);

  void addScene(Scene* scene);
  void focusScene(Scene* scene);
  void render(){focusedScene->renderScene();}
  void focusDirection(unsigned int direction);
  void update();
  bool isFocusingFree(){return !m_focusing_busy;}
  GFXcanvas16 *buffer;

private:
  bool m_focusing_busy = false;
};