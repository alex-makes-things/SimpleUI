#pragma once
#include <ImageAssist.h>
#include <vector>
#include <unordered_map>
#include <UUIDbuddy.h>
#include <Adafruit_GFX.h>
#include <set>

#define RIGHT 0
#define UP 90
#define LEFT 180
#define DOWN 270

using std::string;

//-----------FUNCTION PROTOTYPES----------------//



//-----------FUNCTION PROTOTYPES----------------//

class UI;
class UIElement;
class AnimatedApp;
class Animator;
class UIImage;
struct Point;
struct Cone;
struct Ray;
struct Scene;
struct Focus;
struct FocusingSettings;

struct Point{
  int x=0;
  int y=0;
  Point(int posx=0, int posy=0):x(posx), y(posy){}
  bool operator<(const Point &other) const
  {
    return std::tie(x, y) < std::tie(other.x, other.y);
  }
};

struct Cone{
  unsigned int bisector;        //The angle that indicates the bisector of its aperture (Degrees)
  unsigned int radius;          //How far the cone stretches out (Pixels)
  unsigned int aperture;        //How wide the cone is (Degrees)
  unsigned int aperture_step;   //How many degrees a calculation jumps over the loop of its aperture (Degrees)
  unsigned int rad_step;        //How many pixels a calculation jumps over the loop of its radius (Pixels)
  Cone(unsigned int bisector, unsigned int radius, unsigned int aperture, unsigned int aperture_step, unsigned int rad_step){
    this->bisector = bisector;
    this->radius = radius;
    this->aperture = aperture;
    this->aperture_step = aperture_step;
    this->rad_step = rad_step;
  }
};

struct Ray{
  unsigned int ray_length;
  unsigned int step;
  unsigned int direction;
};

struct Focus{
  std::string focusedElementID;
  std::string previousElementID;
  Scene* focusedScene = nullptr;
  bool isFirstBoot = true;
  inline Focus(std::string ele = "");
  /*!
    @brief Focus an object by its id
    @param sc The scene id to focus
    @param ele The element id to focus
  */
  inline void focus(std::string ele);
  //This function needs to be called at the end of void loop(), which updates the previous focus id to the current one
  inline void update();
  //!@return A boolean that when true, means that the focus has changed in the last cycle
  inline bool hasChanged();
  /*!
    @return A boolean that when true, means that the passed object's identity is currently focused
    @param obj The identity to check if focused
  */
  inline bool isFocusing(std::string obj);
  inline bool isFocusing(UIElement *obj);
};

enum class FocusAccuracy{Low, Medium, High};

struct FocusingSettings{
  unsigned int max_distance;
  bool outline;
  FocusAccuracy accuracy;
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
  inline void setDuration(unsigned int d) { duration = d; }
  //Set the initial value to the passed unsigned integer
  inline void setInitial(float i)
  {
    initial = i;
    progress = (progress < initial) ? initial : progress;
  }
  //Set the final value to the passed unsigned integer
  inline void setFinal(float f)
  {
    final = f;
    progress = (progress > final) ? final : progress;
  }
  //Set the looping setting to the passed bool
  inline void setLoop(bool loop) { looping = loop; }
  //Set the breathing setting to the passed bool
  inline void setBreathing(bool breathe) { breathing = breathe; }
  //Set the reverse setting to the passed bool
  inline void setReverse(bool rev);
  //Starts the animation
  void start();
  //Pauses the animation
  void stop();
  //Switches the enable key, if true it becomes false and viceversa
  void switchState();
  //!@return A float representing the current progress in the animation
  inline float getProgress() { return progress; }
  //!@return True if the animation is at the ending point
  inline bool getDone() { return isDone; }
  //!@return True if the animation is at the starting point
  inline bool getStart() { return isAtStart; }
  //!@return True if the animation is not stopped
  inline bool getIsEnabled() { return enable; }
  //!@return True if the animation is going backwards
  inline bool getDirection() { return reverse; }
  //Resets the animation's settings to the defaults (all false)
  inline void defaultModifiers();
  //Makes the animation restart, works even with reverse
  void resetAnim();
  //Function that can be called at any time which inverts the direction of the animation
  inline void invert();

private:
  bool isAtStart = true;  // If true, the animation is in its starting point
  bool isDone = false;    // Signals if the animation is complete or not
  bool looping = false;   // Makes the animation loop if true
  bool reverse = false;   //"Inverts" the final and initial values, (just in the calculations)
  bool breathing = false; // Gives a breathing effect to the animation, so it animates back to its starting point after reaching its target
  bool bounce_done = false;
  bool enable = false;   // If false, the animation is paused
  unsigned int duration; // How long the animation takes to go from initial to final and viceversa, input as ms but converted to us
  float initial;
  float final;
  float progress;                  // The progress is ultimately the output of the structure, which lies clamped in between initial and final
  uint64_t currentTime = micros(); // This is needed for the temporal aspect of the interpolation
  uint64_t now = micros();         // This is used for synchronicity so that every cycle uses the same time reference
  uint64_t elapsed = 0;            // The amount of time that has passed while the animation was running
};

class UIElement{
  public:
    Animator anim;
    bool centered = false; //If true, the element will be drawn with its center being the coordinates passed to the constructor
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
    UIElement(unsigned int w=0, unsigned int h=0, unsigned int posx=0, unsigned int posy=0);
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    inline void setPosX(unsigned int X) { m_position.x = X; }
    inline void setPosY(unsigned int Y) { m_position.y = Y; }
    inline void setPos(unsigned int X, unsigned int Y){
      m_position.x = X;
      m_position.y = Y;}
    /*!
      @brief Set the UI listener, this allows the element to access its parent UI's attributes and API
      @param listener A pointer to the UI object that "owns" the element
    */
    inline void setUiListener(UI *listener) { m_parent_ui = listener; }
    //!@return The element's UUID
    inline std::string getId() { return m_UUID; }
    inline Point getPos() { return m_position; }
    inline unsigned int getWidth() { return m_width; }
    inline unsigned int getHeight() { return m_height; }
    virtual void render() = 0;
    inline bool isAnimating()
    {
      return !(anim.getDone() || anim.getStart());
    }
    inline bool isFocused();
    inline void setCenter(bool center) { centered = center; }
    Point centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h);

  protected:
    Point m_position;
    bool m_overrideAnimationScaling = false;
    unsigned int m_width=0, m_height=0;
    const std::string m_UUID = RandomBuddy::generateUUID();
    UI* m_parent_ui;
};


class UIImage : public UIElement{
public:
  UIImage(Image &img, unsigned int posx = 0, unsigned int posy = 0) : UIElement(img.width, img.height, posx, posy)
  {
    m_body = &img;
  }

  /// @param scale If less than 0, the scale is controlled by the animation.
  inline void setScale(float scale){m_scale_fac = scale;
                                    m_overrideAnimationScaling = (scale < 0) ? false : true;}
  inline float getScale() { return m_scale_fac; }
  inline void setColor(uint16_t hue) { m_mono_color = hue; }
  inline void setImg(Image *img){m_body = img; m_width = img->width; m_height = img->height;}
  inline Image *getImg() { return m_body; }
  void render() override;

protected:
  Image *m_body;
  float m_scale_fac = 1;
  uint16_t m_mono_color = 0xffff;
};

class AnimatedApp : public UIElement{
  public:
    AnimatedApp(Image& unfocused, Image& focused, int posx, int posy, bool isCentered):UIElement(unfocused.width, unfocused.height, posx, posy){
      centered = isCentered;
      m_unselected = &unfocused;
      m_selected = &focused;
      m_showing = m_unselected;
      m_ratio = static_cast<float>(focused.width) / static_cast<float>(unfocused.width);
      anim = Animator(1, m_ratio, 80);
    }
    
    void render() override;
    inline Image* getActive(){return m_showing;}
    inline void setColor(uint16_t hue){m_mono_color = hue;}
  protected:
    void handleAppSelectionAnimation();
    uint16_t m_mono_color = 0xffff;
    Image* m_unselected = nullptr; //This points to the image that is displayed when the element is unfocused
    Image* m_selected = nullptr;  //This points to the image that is displayed when the element is focused
    Image* m_showing = nullptr;  //This points to the image that is currently being displayed
    float m_ratio;
  };

struct Scene{
  std::string name;
  std::string primaryElementID;
  std::unordered_map<std::string, UIElement*> elements;
  Scene(std::vector<UIElement*> elementGroup, UIElement& first_focus);
  void renderScene();
  inline UIElement* getElementByUUID(std::string UUID);
};

enum class Direction{Up=90, Down=270, Left=180, Right=0};
enum class FocusingType{Linear, Cone};

namespace UiUtils{
  extern const float degToRadCoefficient;
  bool isPointInElement(Point point, UIElement* element);
  Point centerPos(int x_pos, int y_pos, unsigned int w, unsigned int h);
  UIElement* SignedDistance(unsigned int direction, FocusingSettings settings, FocusingType alg, Scene *scene, UIElement *focused);
  Point polarToCartesian(float radius, float angle);
  std::set<Point> computeConePoints(Point vertex, Cone cone);
  UIElement* findElementInCone(UIElement* focused, Scene* currentScene, Cone cone);
  UIElement* findElementInRay(UIElement *focused, Scene *currentScene, Ray ray);
}

class UI{
public:
  Focus focus;
  std::vector<Scene*> scenes;
  FocusingSettings focusingSettings{64, false, FocusAccuracy::Medium};

#ifdef _ADAFRUIT_GFX_H
  UI(Scene& first_scene, GFXcanvas16& framebuffer);
#endif

  void addScene(Scene* scene);
  void focusScene(Scene* scene);
  void render(){focus.focusedScene->renderScene();}
  void focusDirection(unsigned int direction, FocusingType alg);
  void update();
  inline bool isFocusingFree(){return !m_focusing_busy;}

#ifdef _ADAFRUIT_GFX_H
  GFXcanvas16 *buffer;
#endif

private:
  bool m_focusing_busy = false;
};