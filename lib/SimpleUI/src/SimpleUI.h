#pragma once
#include "Texture.h"
#include "UUIDbuddy.h"
#include "Animation.h"
#include <vector>
#include <unordered_map>
#include <Adafruit_GFX.h>
#include <set>
#include <queue>
#include <functional>

#define PERFORMANCE_PROFILING 0
#define LOG(x) Serial.println(x)

#define FPS30 33333
#define FPS60 16667
#define FPS90 11111
#define FPS120 8333
#define FPS144 6944
#define FPS_UNCAPPED 0

#if PERFORMANCE_PROFILING
    #define INSTRUMENTATE(ui) Instrumentator timer(ui, __PRETTY_FUNCTION__);
#else
    #define INSTRUMENTATE(ui)  
#endif

// Index, quickly find all declarations/definitions
namespace SimpleUI
{
  class UI;
  class UIElement;
  class AnimatedApp;
  class UIImage;
  struct Point;
  struct Cone;
  struct Ray;
  struct Scene;
  struct Focus;
  struct FocusingSettings;
  struct Outline;
  enum class Quality;
  enum class Direction;
  enum class FocusingAlgorithm;
  enum class FocusStyle;
  enum class Constraint;
}


namespace SimpleUI{
  
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
  enum class Constraint{
    TopLeft,    Top,      TopRight,

    Left,       Center,   Right,

    BottomLeft, Bottom,   BottomRight
  };

  struct Point{
    int x;   //X coordinate of the point 
    int y;   //Y coordinate of the point
    
    /*!
      @brief Represent a 2D point with integer coordinates.
      @param    x   X coordinate of the point
      @param    y   Y coordinate of the point
    */
    Point(int posx=0, int posy=0) : x(posx), y(posy){};

    bool operator<(const Point &other) const{
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
    Scene* previousScene;
    Scene* activeScene;
    Focus(std::string ele = ""):focusedElementID(ele), activeScene(nullptr), previousScene(nullptr), previousElementID(""){};
    inline void focus(std::string ele);
    inline void update();
    inline bool hasChanged() const;
    inline bool isFocusing(std::string obj);
    inline bool isFocusing(UIElement *obj);
    void focusScene(Scene* scene);
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

  //Generic UI element, all interactable elements inherit from this
  class UIElement{
    public:
      
      Constraint scale_constraint;
      Animation anim;
      FocusStyle focus_style;
      Outline focus_outline;

      bool custom_focus_outline = false;
      bool focusable = true;
      bool draw = true;          //If true, the element is drawn, if false it's kept hidden.

    public:

      UIElement(unsigned int w=0, unsigned int h=0, Point pos={0,0}, bool isCentered = false, ElementType element = ElementType::UIElement, Constraint constraint = Constraint::TopLeft, FocusStyle style = FocusStyle::None)
      : m_type(element), m_width(w), m_height(h), focus_style(style), m_UUID(UUIDbuddy::generateUUID()), m_s_width(w), m_s_height(h), scale_constraint(constraint)
        {
          m_position = isCentered ? centerToCornerPos(pos.x, pos.y, w, h) : pos;
        };

      virtual ~UIElement(){};

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
      inline ElementType getType() const { return m_type; }
      inline Point getPos() const { return m_position; }
      inline unsigned int getWidth() const { return m_width; }
      inline unsigned int getHeight() const { return m_height; }
      inline UI* getParentUI() const { return m_parent_ui; }
      Point getDrawPoint() const;
      Point getCenterPoint() const;
      Point getConstraintedPos() const;
      
      
      virtual void render();
      // Interact with the element
      virtual void click(){return;}
      
      inline bool isAnimating() const {return anim.getState() == AnimState::Running;}
      inline bool isFocused() const;
      
      static Point centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h);
      void drawFocusOutline(const Outline& outline = Outline()) const;

      protected:
      Point m_position;
      bool m_overrideAnimationScaling = false;
      unsigned int m_width, m_height;
      unsigned int m_s_width, m_s_height; //With scaling applied
      std::string m_UUID;
      ElementType m_type;
      UI* m_parent_ui;
  };

  //Used to represent any Image with the tools provided by the library
  class UIImage : public UIElement{
  public:
    UIImage(Texture* img = nullptr, Point pos = {0,0}, bool isCentered=false, FocusStyle focus_style = FocusStyle::None)
    : UIElement(img->width, img->height, pos, isCentered, ElementType::UIImage, Constraint::TopLeft, focus_style), m_body(img), m_mono_color(0xffff), m_scale_fac(1.0f){}

    inline void setScale(float scale){m_scale_fac = scale;
                                      m_overrideAnimationScaling = (scale < 0) ? false : true;}
    inline void setColor(uint16_t hue) { m_mono_color = hue; }
    inline void setImg(Texture *img){m_body = img; m_width = img->width; m_height = img->height;}

    /// @param scale If negative, the scale is controlled by the animation.
    inline float getScale() const { return m_scale_fac; }
    inline Texture *getImg() const { return m_body; }
    

    void render() override;

  protected:
    Texture *m_body;
    float m_scale_fac;
    uint16_t m_mono_color;
  };

  // This is a heavily interactable element which animates from a Texture to another when focused/unfocused, and clicking it can trigger an event
  class AnimatedApp : public UIElement{
    public:

      AnimatedApp(Point pos = {0,0}, bool isCentered = false, Texture* unfocused = nullptr, Texture* focused = nullptr, Constraint constraint = Constraint::TopLeft, 
                  unsigned int duration = 250U, float step = 1.0f):

        UIElement(unfocused ? unfocused->width : 0, unfocused ? unfocused->height : 0, pos, isCentered, ElementType::AnimatedApp, constraint, FocusStyle::Animation),
        m_mono_color(0xffff), m_unselected(unfocused), m_selected(focused), m_showing(m_unselected), m_duration(duration),
        m_ratio(unfocused&&focused ? static_cast<float>(focused->width) / static_cast<float>(unfocused->width) : 0.0f)
      {
        anim = Animation(1.0f, m_ratio, duration, step);
        anim.Start();
        anim.Pause();
      }
      
      void render() override;
      inline Texture* getActive() const {return m_showing;}
      inline void setColor(uint16_t hue){m_mono_color = hue;}
      void click() override{
        m_onClick();
      }
      void bind(const std::function<void()>& func){m_onClick = func;}
      
      
    protected:
      void m_computeAnimation();
      std::function<void()> m_onClick = [](){return;};
    protected:
      unsigned int m_duration;
      uint16_t m_mono_color;  //Color used to draw the textures
      Texture* m_unselected;  //This points to the image that is displayed when the element is unfocused
      Texture* m_selected;    //This points to the image that is displayed when the element is focused
      Texture* m_showing;     //This points to the image that is currently being displayed
      float m_ratio;
    };

  // Extremely customizable yet bare-bones, reliable and easy to work with.
  class Checkbox : public UIElement{

    public:
    Outline outline;           //The checkbox's outline.
    uint16_t selection_color;  //The color of the inside fill when the state is ON
    public:

    /*!
      @brief Create a checkbox element.
      @param style        External outline
      @param pos          Top left corner coordinates
      @param width        Total width in pixels
      @param height       Total height in pixels
      @param fillcolor    RGB565 color of the inside fill
      @param isCentered   Is the element centered around the provided coordinates?
      @param focus_style  What method is used to signal a focus
  */
    Checkbox(Point pos = {0, 0}, bool isCentered=false, unsigned int width=0, unsigned int height=0, Outline style = Outline(),  uint16_t fillColor=0xFFFF, FocusStyle focus_style=FocusStyle::Outline)
      :UIElement(width, height, pos, isCentered, ElementType::Checkbox, Constraint::TopLeft, focus_style), outline(style), selection_color(fillColor){
        focus_outline.border_distance=0U;
        outline.radius = std::clamp(outline.radius, 0U, static_cast<unsigned int>((width >= height ? height : width)*0.5f));
      };
    void render();
    void click() override{m_state = !m_state;}
    /// @return The current state of the checkbox
    inline bool getState() const {return m_state;}

    protected:
    void m_drawCheckboxOutline() const;
    protected:
    bool m_state = false;
  };

  namespace UiUtils{
      constexpr float degToRadCoefficient = 0.01745329251;
      const Point centerPos(int x_pos, int y_pos, const unsigned int w, const unsigned int h);
      
      Point polarToCartesian(const float radius, const float angle);
      bool isPointInElement(Point point, UIElement* element);
      UIElement* SignedDistance(const unsigned int direction, Scene* scene, UIElement* focused);
      UIElement* findElementInCone(UIElement* focused, Scene* currentScene, const Cone& cone);
      UIElement* findElementInRay(UIElement* focused, Scene* currentScene, const Ray& ray);
      const std::string constraintToString(const Constraint constraint);

      std::set<Point> computeConePoints(Point vertex, Cone cone);
    }
  

  //This is one of the most fundamental blocks of the library, it groups together elements and allows for extreme versatility
  class Scene{
    friend class UI;
    friend UIElement* UiUtils::SignedDistance(const unsigned int direction, Scene* scene, UIElement* focused);
    public:
    std::string name;
    std::string primaryElementID;
    std::unordered_map<std::string, UIElement*> elements;
    std::vector<Scene*> parents;

    struct SceneSettings
    {
      struct FocusingSettings
      {
        unsigned int max_distance;
        Quality accuracy;
        FocusingAlgorithm algorithm;
        Outline outline;
      }
      focus{64U, Quality::Medium, FocusingAlgorithm::Linear};

        //Add more stuff here
      bool scriptOnTop = false;
    } 
    settings;

    public:
    Scene(std::initializer_list<UIElement*> elementGroup = {}, UIElement* first_focus = nullptr);
    Scene(const std::function<void()>& script, bool on_top = false) : m_script(script), primaryElementID(""){ settings.scriptOnTop=on_top; }
    void renderScene() const;
    UIElement* getElementByUUID(std::string UUID) const;
    void addParents(std::initializer_list<Scene*> scenes);
    inline void Script(const std::function<void()>& script, bool on_top = false)  { m_script = script; settings.scriptOnTop = on_top;}
    inline void UnbindScript(){ m_script = [](){return;};}
    
    private:
    UI* m_parent_ui;
    std::function<void()> m_script = [](){return;};
  };

  /*This is the object that has the power over the final frame, this reads inputs, handles focusing, and is responsible for calling the rendering
  functions which modify the final buffer.*/
  class UI{
    public:
    Focus focus;
    std::vector<Scene*> scenes;
    GFXcanvas16 *buffer;
    
    
    public:
    UI(Scene* first_scene = nullptr, GFXcanvas16* framebuffer = nullptr);
    void AddScene(Scene* scene);
    void FocusScene(Scene* scene);
    inline const Scene* getActiveScene() const { return focus.activeScene; }
    void Render();
    void FocusDirection(unsigned int direction);
    void FocusDirection(Direction direction);
    void Back();
    void Click();
    inline bool isFocusingFree() const { return !m_focusing_busy; }
    inline UIElement* getFocused() const { return focus.activeScene->getElementByUUID(focus.focusedElementID); }
    
    #if PERFORMANCE_PROFILING
    void printPerfStats();
    private:
    std::unordered_map<std::string, uint32_t> m_perfValues;
    void m_addPerf(std::string key, uint32_t time);
    friend class Instrumentator;
    #endif

    private:
    void m_focusDir(unsigned int direction);
    void m_updateFocus();
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

  
}
