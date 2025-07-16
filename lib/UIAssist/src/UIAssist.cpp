#include <UIAssist.h>

//--------------------Cone STRUCT---------------------------------------------------------------//

/*!
    @brief Initialize a cone
    @param bisector        The angle that indicates the bisector of its aperture (Degrees)
    @param radius          How far the cone stretches out (Pixels)
    @param aperture        How wide the cone is (Degrees)
    @param aperture_step   How many degrees a calculation jumps over the loop of its aperture (Degrees)
    @param rad_step        How many pixels a calculation jumps over the loop of its radius (Pixels)
  */
Cone::Cone(unsigned int bisector, unsigned int radius, unsigned int aperture, unsigned int aperture_step, unsigned int rad_step)
    :bisector(bisector), radius(radius), aperture(aperture), aperture_step(aperture_step), rad_step(rad_step){}

/*!
    @brief Represent a 2D point with integer coordinates.
    @param    x   X coordinate of the point
    @param    y   Y coordinate of the point
*/
Point::Point(int posx, int posy) : x(posx), y(posy){}
//--------------------FOCUS STRUCT---------------------------------------------------------------//

Focus::Focus(std::string ele):focusedElementID(ele){}

/*!
    @brief Focus an object by its UUID
    @param ele The element UUID to focus
*/
void Focus::focus(std::string ele){
    previousElementID = focusedElementID;
    focusedElementID = ele;
  }

//This function needs to be called at the end of a cycle, which updates the previous focus id to the current one
void Focus::update(){
    previousElementID = focusedElementID;
    isFirstBoot = false;
  }

//!@return A boolean that when true, means that the focus has changed in the last cycle
bool Focus::hasChanged(){
    return (previousElementID != focusedElementID);
  }

/*!
  @return A boolean that when true, means that the passed object's identity is currently focused
  @param obj The UUID of the object in cause
*/
bool Focus::isFocusing(std::string obj){
    return (focusedElementID == obj);
  }

/*!
  @return A boolean that when true, means that the passed object's identity is currently focused
  @param obj The pointer of the object in cause
*/
bool Focus::isFocusing(UIElement* obj){
    return (focusedElementID == obj->getId());
  }

//--------------------ANIMATOR CLASS---------------------------------------------------------------//
Animator::Animator(float i, float f, unsigned int d){  //Basic constructor
    duration = d*1000;
    initial = i;
    final = f;
    progress = initial;
  }

void Animator::update(){
    if(reverse){
      //Magnetic attachment to initial value and setting isAtStart
        
        if (fabs(progress-final)<=EPSILON){
          progress = final;
          isAtStart =true;
        }
      }else{
        if (fabs(progress-initial)<=EPSILON){
          progress = initial;
          isAtStart =true;
        }
      }

    //If the animation is active, the update logic runs
    if (enable){
      now = micros();
      elapsed = std::clamp(now-currentTime, 0ULL, static_cast<uint64_t>(duration));

      //Here I add anything that has to run when the animation is in its done state
      if(isDone){
        //Breathing feature
        if(breathing){
          if(looping){
            invert();
            bounce_done = false;
          } else if (!bounce_done){
            //invert();
            //isDone=false;
            resetAnim();
            reverse = !reverse;
            bounce_done = true;
          }
        }
        //Restarts the animation if the looping condition is met
        if(looping){
          resetAnim();
        }
      }

      //Interpolation
      else{
        if (elapsed<duration){
          progress = std::clamp(Fmap((reverse) ? duration-elapsed : elapsed, 0, duration, initial, final), initial, final);
        } else {
          isDone = true;
          progress = (reverse) ? initial : final;
        }
        isAtStart = false;
      }
    }
  }


void Animator::setReverse(bool rev){
    reverse = rev;
    progress = (progress==initial&&reverse) ? final : progress;
  }

void Animator::stop(){
    update();
    enable = false;
  }

void Animator::switchState(){
    if (!enable){
      currentTime = micros()-elapsed;
    }else{update();}
    enable = !enable;
  }

void Animator::start(){
    enable = true;
    currentTime = micros()-elapsed;
  }
  
void Animator::defaultModifiers(){
    looping = false;
    reverse = false;
    breathing = false;
    bounce_done = false;
  }

void Animator::resetAnim(){
    progress = (reverse) ? final : initial;
    currentTime = micros();
    isDone = false;
    isAtStart = true;
  }

void Animator::invert(){ 
    reverse = !reverse;
    elapsed = duration-elapsed;
    currentTime = now-elapsed;
    now = currentTime;
  }

//--------------------UIElement CLASS---------------------------------------------------------------//

UIElement::UIElement(unsigned int w, unsigned int h, Point pos, ElementType element, FocusStyle style) 
: type(element), m_width(w), m_height(h), m_position(pos), focus_style(style){}

Point UIElement::centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h) const {
      unsigned int new_x = static_cast<unsigned int>(static_cast<float>(x_pos)-(static_cast<float>(w)*0.5f));
      unsigned int new_y = static_cast<unsigned int>(static_cast<float>(y_pos)-(static_cast<float>(h)*0.5f));
      return Point(new_x, new_y);
    }

bool UIElement::isFocused(){
  return m_parent_ui->focus.focusedElementID==m_UUID;
}

void UIElement::drawFocusOutline(){
  if (focus_style == FocusStyle::Outline && isFocused()) {

    Point rect_drawing_pos = getDrawPoint();
    rect_drawing_pos -= (focus_outline.border_distance + 1);
    int16_t draw_width = m_width + focus_outline.border_distance*2 +2;
    int16_t draw_height = m_height + focus_outline.border_distance*2 +2;


    if(focus_outline.radius != 0){
      int16_t draw_radius = focus_outline.radius;
      for (int i = 0; i < focus_outline.thickness; i++) {
        m_parent_ui->buffer->drawRoundRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, draw_radius, focus_outline.color);
        if (!(i % 2)){
          int16_t temp_r = draw_radius + 1;
          m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + temp_r, temp_r, 1, focus_outline.color);
          m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + temp_r, temp_r, 2, focus_outline.color);
          m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 4, focus_outline.color);
          m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 8, focus_outline.color);
        }
        
        draw_width += 2;
        draw_height += 2; 
        rect_drawing_pos--;
        draw_radius++;
      }
    }
    else{

      for (int i = 0; i < focus_outline.thickness; i++) {
        m_parent_ui->buffer->drawRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, focus_outline.color);
        draw_width += 2;
        draw_height += 2;
        rect_drawing_pos--;
      }

    }
  }
}

void UIElement::render(){
  drawFocusOutline();
}

Point UIElement::getDrawPoint() const {
  return centered ? centerToCornerPos(m_position.x, m_position.y, m_width, m_height) : m_position;
}

Point UIElement::getCenterPoint() const {
  return centered ? m_position : UiUtils::centerPos(m_position.x, m_position.y, m_width, m_height);
}

//--------------------UIImage CLASS---------------------------------------------------------------//

void UIImage::render(){
  drawFocusOutline();
  anim.update();
  

  const float m_scale_fac = (m_overrideAnimationScaling) ? m_scale_fac : anim.getProgress();
  Image drawing_image = m_scale_fac != 1 ? scale(*m_body, m_scale_fac) : *m_body;
  Point drawing_pos = centered ? centerToCornerPos(m_position.x, m_position.y, drawing_image.width, drawing_image.height) : m_position;
  if(drawing_image.data.colorspace == PixelType::Mono){
    m_parent_ui->buffer->drawBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.mono, drawing_image.width, drawing_image.height, m_mono_color);
  }
  else{
    m_parent_ui->buffer->drawRGBBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.rgb565, drawing_image.width, drawing_image.height);
  }

}

//--------------------AnimatedApp CLASS---------------------------------------------------------------//

void AnimatedApp::m_computeAnimation(){
  if (m_parent_ui->focus.hasChanged() || (m_parent_ui->focus.isFirstBoot && isFocused()))
  {
    if(isFocused()){  
    //If the element is being focused

      if(isAnimating() && anim.getDirection()){
        anim.invert();                         // If it's mid closing-animation, invert it
      }
      if(getActive()==m_unselected && anim.getStart())
      { //If it's showing the small img and hasn't started

        anim.start();
      }
    }
    else{ //Focus has changed but the element isn't focused
      if(getActive()==m_selected && anim.getDone() && !anim.getDirection())
      {
        anim.setFinal(m_ratio);
        m_showing = m_unselected;
        anim.resetAnim();
        anim.setReverse(true);
      }
      if(isAnimating() && !anim.getDirection()){
        anim.invert();                          // If it's mid opening-animation, invert it
      }
    }
  }
  else{
    if (getActive()==m_unselected && anim.getDone()){
      if (!anim.getDirection()){
        //Since we animated from a small image to a larger image, setting final to 1 automatically clamps it to 1
        anim.setFinal(1); 
        m_showing = m_selected;
      }
      else //This branch however, only runs on the falling part of the animation
      {
        //After the sum of all the conditions, we know that the animation has finished reversing to the smaller image
        anim.setFinal(m_ratio);
        anim.setReverse(false);  //Make sure the animation goes in the right direction when it is focused again
        anim.resetAnim();
        anim.stop();   
      }
    }
    
    if(!isFocused() && !isAnimating() && m_showing == m_selected){
      anim.setFinal(m_ratio);
      m_showing = m_unselected;
      anim.resetAnim();
      anim.setReverse(true);
    }
    if (isFocused() && anim.getStart() && !anim.getIsEnabled() && m_showing == m_unselected){
      anim.start();
    }
  }
}

void AnimatedApp::render(){
  anim.update();
  if (focus_style == FocusStyle::Animation)
    m_computeAnimation();
 
  const float scale_fac = anim.getProgress();
  Image drawing_image = scale_fac != 1 ? scale(*m_showing, scale_fac) : *m_showing;
  Point drawing_pos = centered ? centerToCornerPos(m_position.x, m_position.y, drawing_image.width, drawing_image.height) : m_position;
  if(drawing_image.data.colorspace == PixelType::Mono){
    m_parent_ui->buffer->drawBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.mono, drawing_image.width, drawing_image.height, m_mono_color);
  }
  else{
    m_parent_ui->buffer->drawRGBBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.rgb565, drawing_image.width, drawing_image.height);
  }

}

//--------------------Checkbox CLASS---------------------------------------------------------------//



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
Checkbox::Checkbox(Outline style, Point pos, unsigned int width, unsigned int height, uint16_t fillColor,bool isCentered, FocusStyle focus_style)
    :UIElement(width, height, pos, ElementType::Checkbox, focus_style), outline(style), selection_color(fillColor){
      centered = isCentered;
      focus_outline.border_distance=0U;
      outline.radius = std::clamp(outline.radius, 0U, static_cast<unsigned int>((width >= height ? height : width)*0.5f));
    }

void Checkbox::m_drawCheckboxOutline(){
 
  int16_t draw_width = m_width;
  int16_t draw_height = m_height;
  Point rect_drawing_pos = getDrawPoint();

  if(outline.radius!=0){
    int16_t draw_radius = outline.radius+outline.thickness;
    for (int i = outline.thickness; i > 0 ; i--) {
      m_parent_ui->buffer->drawRoundRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, draw_radius, outline.color);
      if (!(i % 2)){
        int16_t temp_r = draw_radius + 1;
        m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + temp_r, temp_r, 1, outline.color);
        m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + temp_r, temp_r, 2, outline.color);
        m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 4, outline.color);
        m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 8, outline.color);
      }
      
      draw_width -= 2;
      draw_height -= 2; 
      rect_drawing_pos++;
      draw_radius--;
    }
  }
  else{
    
    for (int i = outline.thickness; i > 0 ; i--) {
      m_parent_ui->buffer->drawRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, outline.color);
      draw_width -= 2;
      draw_height -= 2;
      rect_drawing_pos++;
    }
  }
}

void Checkbox::render(){
  m_drawCheckboxOutline();
  if(m_state){
    Point fill_pos = getDrawPoint();
    fill_pos += (outline.border_distance+outline.thickness);
    const unsigned int offset=outline.border_distance*4;
    if(outline.radius!=0){
      m_parent_ui->buffer->fillRoundRect(fill_pos.x, fill_pos.y, m_width-offset, m_height-offset, outline.radius-outline.border_distance, selection_color);
    }
    else{
      m_parent_ui->buffer->fillRect(fill_pos.x, fill_pos.y, m_width-offset, m_height-offset, selection_color);
    }
  }
}


//--------------------Scene STRUCT---------------------------------------------------------------//

Scene::Scene(std::vector<UIElement*> elementGroup, UIElement& first_focus){
    for(int i = 0; i<elementGroup.size(); i++){
      elements.insert({elementGroup[i]->getId(),elementGroup[i]});
    }
    primaryElementID = first_focus.getId();
  }

void Scene::renderScene()
  {
    for (const auto&[id, element] : elements)
    {
      if(element->draw){
        element->render();
        if(element->isFocused()&&element->focus_style==FocusStyle::Outline)
          element->drawFocusOutline();
      }
    }
  }

UIElement* Scene::getElementByUUID(std::string UUID){
  return elements.at(UUID);
}

//--------------------UI CLASS---------------------------------------------------------------//

UI::UI(Scene& first_scene, GFXcanvas16& framebuffer)
{
  focus = Focus(first_scene.primaryElementID);
  buffer = &framebuffer;
  addScene(first_scene);
  focusScene(first_scene);
}

void UI::addScene(Scene& scene){
    scenes.push_back(&scene);                 //KEEP IN MIND "REALLOCATES"
    for(const auto&[id, element] : scene.elements){
      element->setUiListener(this);
    }
  }

void UI::focusScene(Scene& scene){
    focus.focusedScene = &scene;
    focus.focusedElementID = focus.focusedScene->primaryElementID;
  }



/// @brief Focus the closest object in any direction
/// @param direction The direction in counter clockwise degrees, with its origin being the center of the currently focused element (Right is 0)
/// @param alg The focusing algorithm that you want to use (FocusingAlgorithm::Linear, FocusingAlgorithm::Cone)
void UI::focusDirection(unsigned int direction, FocusingAlgorithm alg){
  if(isFocusingFree()){
    m_focusing_busy = true;
    UIElement* next_element = UiUtils::SignedDistance(direction, focusingSettings, alg, focus.focusedScene, focus.focusedScene->getElementByUUID(focus.focusedElementID));
    if(next_element){
        focus.focus(next_element->getId());
        focus.focusedElementID = next_element->getId();
    }
  }
}

void UI::update(){
  focus.update();
  m_focusing_busy = false;
}

//--------------------UiUtils NAMESPACE---------------------------------------------------------------//


bool UiUtils::isPointInElement(Point point, UIElement* element){
    const Point element_pos = element->getPos();
    if ((point.x >= element_pos.x && point.x <= element_pos.x + element->getWidth()) && (point.y >= element_pos.y && point.y <= element_pos.y + element->getHeight())){
      return true;
    }
    return false;
  }

/*!
    @param x_pos  X coordinate of the top-left corner
    @param y_pos  Y coordinate of the top-left corner
    @param w      Width in pixels
    @param h      Height in pixels
    @return The center point of the described boundary
  */
const Point UiUtils::centerPos(int x_pos, int y_pos, const unsigned int w, const unsigned int h){
    int new_x = round(static_cast<float>(x_pos)+(w*0.5f));
    int new_y = round(static_cast<float>(y_pos)+(h*0.5f));
    return Point(new_x, new_y);
  }

UIElement* UiUtils::SignedDistance(unsigned int direction, FocusingSettings& settings, FocusingAlgorithm& alg, Scene*& scene, UIElement* focused){
    if (alg == FocusingAlgorithm::Linear)
    {
      Ray ray{settings.max_distance, 1, direction};
      switch (settings.accuracy)
      {
      case Quality::Low:
        ray.step = 4;
        break;
      case Quality::Medium:
        ray.step = 2;
        break;
      case Quality::High:
        ray.step = 1;
        break;
      }
      return findElementInRay(focused, scene, ray);
    }

    else  //IF THE METHOD IS CONE
    {
      Cone cone(direction, settings.max_distance, 90, 2, 6);
      switch (settings.accuracy){
        case Quality::Low:
          cone.aperture_step = 3;
          cone.rad_step = 8;
          break;
        case Quality::Medium:
          cone.aperture_step = 2;
          cone.rad_step = 6;
          break;
        case Quality::High:
          cone.aperture_step = 1;
          cone.rad_step = 2;
          break;
        }
      return findElementInCone(focused, scene, cone);
    }

    return nullptr;
}

Point UiUtils::polarToCartesian(const float radius, const float angle){
  return Point(radius * cos(-angle * degToRadCoefficient), //x
               radius * sin(-angle * degToRadCoefficient));//y
}

std::set<Point> UiUtils::computeConePoints(Point vertex, Cone cone){
  std::set<Point> buffer;

  int half_aperture = cone.aperture/2;
  int starting_angle = cone.bisector - half_aperture;
  int end_angle = cone.bisector + half_aperture;

  for(int i = starting_angle; i<end_angle; i+=cone.aperture_step){
    for(int b = 0; b<cone.radius; b+=cone.rad_step){
      Point tempPoint = polarToCartesian(b, i);
      tempPoint.x += vertex.x;
      tempPoint.y += vertex.y;
      buffer.emplace(tempPoint);
    }
  }
  return buffer;
}

UIElement* UiUtils::findElementInCone(UIElement*& focused, Scene*& currentScene, const Cone& cone){
  focused->focusable = false;

  const int half_aperture = static_cast<int>(cone.aperture * 0.5);
  const int starting_angle = cone.bisector - half_aperture;
  const int end_angle = cone.bisector + half_aperture;
  const Point centerPoint = focused->getCenterPoint();


  Point tempPoint;
  for (int i = starting_angle; i < end_angle; i += cone.aperture_step)
  {
    for (int b = 0; b < cone.radius; b += cone.rad_step)
    {
      tempPoint = polarToCartesian(b, i) + centerPoint;
      for(const auto&[id, element] : currentScene->elements){
        if (element->focusable && isPointInElement(tempPoint, element)){
          focused->focusable = true;
          return element;
        }
      }
    }
  }
  focused->focusable = true;
  return nullptr;
}

UIElement* UiUtils::findElementInRay(UIElement*& focused, Scene*& currentScene, const Ray& ray){
  focused->focusable = false;
  const Point centerPoint = focused->getCenterPoint();
  Point tempPoint;
  
  for(int i = 0; i<ray.ray_length; i+=ray.step){
    tempPoint = polarToCartesian(i, ray.direction) + centerPoint;
    for(const auto&[id, element] : currentScene->elements){
      if (element->focusable && isPointInElement(tempPoint, element)){
        focused->focusable = true;
        return element;
      }
    }
  }
  focused->focusable = true;
  return nullptr;
}