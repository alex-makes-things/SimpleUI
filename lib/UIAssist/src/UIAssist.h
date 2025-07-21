#pragma once
#include <Image.h>
#include <vector>
#include <unordered_map>
#include <UUIDbuddy.h>
#include <Adafruit_GFX.h>
#include <set>
#include <functional>

#define PERFORMANCE_PROFILING 0

#define RIGHT 0
#define UP 90
#define LEFT 180
#define DOWN 270

#if PERFORMANCE_PROFILING
    #define INSTRUMENTATE(ui) Instrumentator timer(ui, __PRETTY_FUNCTION__);
#else
    #define INSTRUMENTATE(ui)  
#endif

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
struct Outline;

enum class ElementType{
  UIElement,
  AnimatedApp,
  UIImage,
  Checkbox
};
enum class Quality{Low, Medium, High};
enum class Direction{Up=90, Down=270, Left=180, Right=0};
enum class FocusingAlgorithm{Linear, Cone};
enum class FocusStyle{None, Animation, Outline, Color};


struct Point{
  int x=0;   //X coordinate of the point 
  int y=0;   //Y coordinate of the point
  
  Point(int posx=0, int posy=0);

  bool operator<(const Point &other) const
  {
    return std::tie(x, y) < std::tie(other.x, other.y);
  }
  Point& operator+=(int value) {
    x += value;
    y += value;
    return *this;
  }
  Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  Point& operator++(int) {
    x++;
    y++;
    return *this;
  }
  Point& operator--(int) {
    x--;
    y--;
    return *this;
  }
  Point& operator-=(int value) {
    x -= value;
    y -= value;
    return *this;
  }
  Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }
};

// Holds the parameters necessary for computing a 2D cone with whatever level of detail desired
struct Cone{
  unsigned int bisector;        //The angle that indicates the bisector of its aperture (Degrees)
  unsigned int radius;          //How far the cone stretches out (Pixels)
  unsigned int aperture;        //How wide the cone is (Degrees)
  unsigned int aperture_step;   //How many degrees a calculation jumps over the loop of its aperture (Degrees)
  unsigned int rad_step;        //How many pixels a calculation jumps over the loop of its radius (Pixels)
  Cone(unsigned int bisector, unsigned int radius, unsigned int aperture, unsigned int aperture_step, unsigned int rad_step);
};

// Holds the parameters necessary for computing a 2D ray with whatever level of detail desired
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
  inline void focus(std::string ele);
  inline void update();
  inline bool hasChanged();
  inline bool isFocusing(std::string obj);
  inline bool isFocusing(UIElement *obj);
};

struct FocusingSettings{
  unsigned int max_distance;
  Quality accuracy;
};

struct Outline{
  unsigned int thickness; 
  unsigned int border_distance;
  unsigned int radius;
  uint16_t color;
  /*!
    @brief Represent any type of outline, used in different ways by each function.
    @param thickness  How many layers the outline has in pixels
    @param distance   Distance in pixels from the outline to the outlined element
    @param radius     Radius in pixels of the corners
    @param color      RGB565 color of the outline
*/
  Outline(unsigned int thickness=1, unsigned int distance=0, unsigned int radius = 0, uint16_t color=0xffff) : thickness(thickness), border_distance(distance), color(color), radius(radius){}
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
  inline float getProgress() const { return progress; }
  //!@return True if the animation is at the ending point
  inline bool getDone() const { return isDone; }
  //!@return True if the animation is at the starting point
  inline bool getStart() const { return isAtStart; }
  //!@return True if the animation is not stopped
  inline bool getIsEnabled() const { return enable; }
  //!@return True if the animation is going backwards
  inline bool getDirection() const { return reverse; }
  //Resets the animation's settings to the defaults (all false)
  inline void defaultModifiers();
  //Makes the animation restart, works even with reverse
  void resetAnim();
  //Function that can be called at any time which inverts the direction of the animation
  inline void invert();

private:
  static constexpr float EPSILON = 0.0005f;
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
  uint32_t currentTime = micros(); // This is needed for the temporal aspect of the interpolation
  uint32_t now = micros();         // This is used for synchronicity so that every cycle uses the same time reference
  uint32_t elapsed = 0;            // The amount of time that has passed while the animation was running
};

//Generic UI element, all interactable elements inherit from this
class UIElement{
  public:
    bool focusable = true;
    Animator anim;
    const ElementType type;
    FocusStyle focus_style;
    Outline focus_outline;
    bool centered = false; //If true, the element will be drawn with its center being the coordinates passed to the constructor
    bool draw=true; //If true, the element is drawn, if false it's kept hidden.
  public:
    UIElement(unsigned int w=0, unsigned int h=0, Point pos = {0,0}, ElementType element = ElementType::UIElement, FocusStyle style = FocusStyle::None);
    virtual ~UIElement(){};
    void InitAnim(float initial, float final, unsigned int duration){anim = Animator(initial, final, duration);}
    inline void setPosX(unsigned int X) { m_position.x = X; }
    inline void setPosY(unsigned int Y) { m_position.y = Y; }
    inline void setPos(Point pos){m_position=pos;}
    /*!
      @brief Set the UI listener, this allows the element to access its parent UI's attributes and API
      @param listener A pointer to the UI object that "owns" the element
    */
    inline void setUiListener(UI *listener) { m_parent_ui = listener; }
    //!@return The element's UUID
    inline std::string getId() const { return m_UUID; }
    inline Point getPos() const { return m_position; }
    inline unsigned int getWidth() const { return m_width; }
    inline unsigned int getHeight() const { return m_height; }
    inline UI* getParentUI() const { return m_parent_ui; }
    virtual void render();
    // Interact with the element
    virtual void click(){return;}
    inline bool isAnimating() const {return !(anim.getDone() || anim.getStart());}
    inline bool isFocused() const;
    inline void setCenter(bool center) { centered = center; }
    Point centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h) const ;
    Point getDrawPoint() const;
    Point getCenterPoint() const;
    void drawFocusOutline() const;
  protected:
    Point m_position;
    bool m_overrideAnimationScaling = false;
    unsigned int m_width=0, m_height=0;
    const std::string m_UUID = RandomBuddy::generateUUID();
    UI* m_parent_ui;
};

//Used to represent any Image with the tools provided by the library
class UIImage : public UIElement{
public:
  UIImage(Texture &img, Point pos, FocusStyle focus_style = FocusStyle::None): UIElement(img.width, img.height, pos, ElementType::UIImage, focus_style), m_body(&img){}
  /// @param scale If negative, the scale is controlled by the animation.
  inline void setScale(float scale){m_scale_fac = scale;
                                    m_overrideAnimationScaling = (scale < 0) ? false : true;}
  inline float getScale() const { return m_scale_fac; }
  inline void setColor(uint16_t hue) { m_mono_color = hue; }
  inline void setImg(Texture *img){m_body = img; m_width = img->width; m_height = img->height;}
  inline Texture *getImg() const { return m_body; }
  void render() override;

protected:
  Texture *m_body;
  float m_scale_fac = 1;
  uint16_t m_mono_color = 0xffff;
};

// This is a heavily interactable element which animates from a Texture to another when focused/unfocused, and clicking it can trigger an event
class AnimatedApp : public UIElement{
  public:
    AnimatedApp(Texture& unfocused, Texture& focused, Point pos, bool isCentered, FocusStyle focus_style):UIElement(unfocused.width, unfocused.height, pos, ElementType::AnimatedApp, focus_style){
      centered = isCentered;
      m_unselected = &unfocused;
      m_selected = &focused;
      m_showing = m_unselected;
      m_ratio = static_cast<float>(focused.width) / static_cast<float>(unfocused.width);
      anim = Animator(1, m_ratio, 75);
    }
    
    void render() override;
    inline Texture* getActive(){return m_showing;}
    inline void setColor(uint16_t hue){m_mono_color = hue;}
    void click() override{
      m_onClick();
    }
    void bind(const std::function<void()>& func){m_onClick = func;}
  protected:
    void m_computeAnimation();
    std::function<void()> m_onClick = [](){return;};
    uint16_t m_mono_color = 0xffff;
    Texture* m_unselected = nullptr; //This points to the image that is displayed when the element is unfocused
    Texture* m_selected = nullptr;  //This points to the image that is displayed when the element is focused
    Texture* m_showing = nullptr;  //This points to the image that is currently being displayed
    float m_ratio;
  };

// Extremely customizable yet bare-bones, reliable and easy to work with.
class Checkbox : public UIElement{
  public:
  Outline outline;           //The checkbox's outline.
  uint16_t selection_color;  //The color of the inside fill when the state is ON
  public:
  Checkbox(Outline style, Point pos, unsigned int width=0, unsigned int height=0, uint16_t fillColor=0xFFFF, bool isCentered=false, FocusStyle focus_style=FocusStyle::Outline);
  void render();
  void click() override{m_state = !m_state;}
  /// @return The current state of the checkbox
  inline bool getState() const {return m_state;}

  protected:
  void m_drawCheckboxOutline() const;
  protected:
  bool m_state=false;
};

//This is one of the most fundamental blocks of the library, it groups together elements and allows for extreme versatility
struct Scene{
  std::string name;
  std::string primaryElementID;
  std::unordered_map<std::string, UIElement*> elements;
  Scene(std::vector<UIElement*> elementGroup, UIElement& first_focus);
  void renderScene() const;
  UIElement* getElementByUUID(std::string UUID) const;
};

namespace UiUtils{
  constexpr float degToRadCoefficient = 0.01745329251;
  const Point centerPos(int x_pos, int y_pos, const unsigned int w, const unsigned int h);
  
  Point polarToCartesian(const float radius, const float angle);
  bool isPointInElement(Point point, UIElement* element);
  UIElement* SignedDistance(const unsigned int direction, const FocusingSettings& settings, const FocusingAlgorithm& alg, Scene* scene, UIElement* focused);
  UIElement* findElementInCone(UIElement* focused, Scene* currentScene, const Cone& cone);
  UIElement* findElementInRay(UIElement* focused, Scene* currentScene, const Ray& ray);

  std::set<Point> computeConePoints(Point vertex, Cone cone);
}

/*This is the object that has the power over the final frame, this reads inputs, handles focusing, and is responsible for calling the rendering
functions which modify the final buffer.*/
class UI{
  public:
  Focus focus;
  std::vector<Scene*> scenes;
  FocusingSettings focusingSettings{64, Quality::Medium};
  GFXcanvas16 *buffer;
  
  
  public:
  UI(Scene& first_scene, GFXcanvas16& framebuffer);
  void addScene(Scene& scene);
  void focusScene(Scene& scene);
  void render(){focus.focusedScene->renderScene();}
  void focusDirection(unsigned int direction, FocusingAlgorithm alg);
  void update();
  inline bool isFocusingFree() const { return !m_focusing_busy; }
  
  #if PERFORMANCE_PROFILING
    void printPerfStats();
    std::unordered_map<std::string, uint32_t> m_perfValues;
    void m_addPerf(std::string key, uint32_t time);
  #endif


  private:
  bool m_focusing_busy = false; //You could see this as sort of a "mutex" to prevent multiple focuses from happening in the same cycle, which could break a UI
};

#if PERFORMANCE_PROFILING
class Instrumentator{
  UI* target = nullptr;
  const std::string funcName;
  const uint32_t start;
  public:
  Instrumentator(UI* target, std::string func) : target(target), funcName(func), start(micros()){}
  ~Instrumentator(){
    target->m_addPerf(funcName, micros()-start);
  }

};
#endif