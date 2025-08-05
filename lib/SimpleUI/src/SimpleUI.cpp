#include "SimpleUI.h"

namespace SimpleUI{
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


//--------------------FOCUS STRUCT---------------------------------------------------------------//


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
    }

  //!@return A boolean that when true, means that the focus has changed in the current cycle
  bool Focus::hasChanged() const {
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

  void Focus::focusScene(Scene* scene){
    if (scene){
      previousScene = activeScene;
      activeScene = scene;
      focus(scene->primaryElementID);
    }
  }

//--------------------UIElement CLASS---------------------------------------------------------------//

  Point UIElement::centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h){
        unsigned int new_x = static_cast<unsigned int>(static_cast<float>(x_pos)-(static_cast<float>(w)*0.5f));
        unsigned int new_y = static_cast<unsigned int>(static_cast<float>(y_pos)-(static_cast<float>(h)*0.5f));
        return Point(new_x, new_y);
      }

  bool UIElement::isFocused() const {
    return m_parent_ui->focus.focusedElementID==m_UUID;
  }

  void UIElement::drawFocusOutline(const Outline& outline) const {
    INSTRUMENTATE(m_parent_ui)
    if (focus_style == FocusStyle::Outline && isFocused()) {
      Outline draw_outline = custom_focus_outline ? focus_outline : outline;

      Point rect_drawing_pos = getDrawPoint();
      rect_drawing_pos -= (draw_outline.border_distance + 1);
      int16_t draw_width = m_width + draw_outline.border_distance*2 +2;
      int16_t draw_height = m_height + draw_outline.border_distance*2 +2;


      if(draw_outline.radius != 0){
        int16_t draw_radius = draw_outline.radius;
        for (int i = 0; i < draw_outline.thickness; i++) {
          m_parent_ui->buffer->drawRoundRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, draw_radius, draw_outline.color);
          if (!(i % 2)){
            int16_t temp_r = draw_radius + 1;
            m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + temp_r, temp_r, 1, draw_outline.color);
            m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + temp_r, temp_r, 2, draw_outline.color);
            m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + draw_width - temp_r - 1, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 4, draw_outline.color);
            m_parent_ui->buffer->drawCircleHelper(rect_drawing_pos.x + temp_r, rect_drawing_pos.y + draw_height - temp_r - 1, temp_r, 8, draw_outline.color);
          }
          
          draw_width += 2;
          draw_height += 2; 
          rect_drawing_pos--;
          draw_radius++;
        }
      }
      else{

        for (int i = 0; i < draw_outline.thickness; i++) {
          m_parent_ui->buffer->drawRect(rect_drawing_pos.x, rect_drawing_pos.y, draw_width, draw_height, draw_outline.color);
          draw_width += 2;
          draw_height += 2;
          rect_drawing_pos--;
        }

      }
    }
  }

  void UIElement::render(){
    INSTRUMENTATE(m_parent_ui)
    drawFocusOutline();
  }

  Point UIElement::getDrawPoint() const {
    return getConstraintedPos();
  }

  Point UIElement::getCenterPoint() const {
    return UiUtils::centerPos(m_position.x, m_position.y, m_width, m_height);
  }

  Point UIElement::getConstraintedPos() const {
    int widthDiff = m_width - m_s_width;
    int heightDiff = m_height - m_s_height;
    int widthDiff_2 = static_cast<int>(widthDiff * 0.5);
    int heightDiff_2 = static_cast<int>(heightDiff * 0.5);
    switch (scale_constraint){
      case Constraint::TopLeft:     return Point( m_position.x, m_position.y );
      case Constraint::Top:         return Point( m_position.x + widthDiff_2,  m_position.y);
      case Constraint::TopRight:    return Point( m_position.x + widthDiff,  m_position.y);
      case Constraint::Left:        return Point( m_position.x,  m_position.y + heightDiff_2);
      case Constraint::Center:      return Point( m_position.x + widthDiff_2,  m_position.y + heightDiff_2);
      case Constraint::Right:       return Point( m_position.x + widthDiff,  m_position.y + heightDiff_2);
      case Constraint::BottomLeft:  return Point( m_position.x,  m_position.y + heightDiff);
      case Constraint::Bottom:      return Point( m_position.x + widthDiff_2,  m_position.y + heightDiff);
      case Constraint::BottomRight: return Point( m_position.x + heightDiff,  m_position.y + heightDiff);
    }
    return Point(m_position.x, m_position.y);
  }

//--------------------UIImage CLASS---------------------------------------------------------------//

  void UIImage::render(){
    INSTRUMENTATE(m_parent_ui)
    drawFocusOutline();
    anim.Update();
  
    const float scale_fac = anim.getProgress();
    const Texture drawing_image = scale(*m_body, scale_fac);
    m_s_height = drawing_image.height;
    m_s_width = drawing_image.width;
    const Point drawing_pos = getConstraintedPos();

    if(drawing_image.data.colorspace == PixelType::Mono){
      m_parent_ui->buffer->drawBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.mono, drawing_image.width, drawing_image.height, m_mono_color);
    }
    else{
      m_parent_ui->buffer->drawRGBBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.rgb565, drawing_image.width, drawing_image.height);
    }

  }

//--------------------AnimatedApp CLASS---------------------------------------------------------------//
  
  void AnimatedApp::m_computeAnimation(){
    INSTRUMENTATE(m_parent_ui)
    switch(anim.getState()){

      case AnimState::Start:{
        if (isFocused())
        {
          if (m_showing==m_unselected) 
            anim.Resume();
        }
        else if (m_parent_ui->focus.hasChanged() && m_showing == m_selected)
        { //If the element has just been unfocused and has previously completed the focusing animation, start the unfocusing
          m_showing = m_unselected;
          anim = Animation(m_ratio, 1.0f, m_duration, m_func);
          anim.Start();
        }
        break;
      }
      case AnimState::Running:{ //Flip the animation when interacting to create a smooth ascend/descend mid-animation
        if(m_parent_ui->focus.hasChanged())
        {
          if(anim.getDirection() && !isFocused())
            anim.Flip();
          else if(!anim.getDirection() && isFocused())
            anim.Flip();
        }
        break;
      }
      case AnimState::Finished:{
        if ( m_showing == m_unselected)
        {
          if (anim.getDirection())
          {
            if (isFocused())
            { //Set the current progress to 1 for correct scaling of focused icon
              m_showing = m_selected;
              anim.Reset();
              anim.Pause();
            }
            else
            { //Fixes bug that causes the unfocused icon to stay big while it isn't focused
              anim = Animation(m_ratio, 1.0f, m_duration, m_func);
              anim.Start();
            }
          }
          else { //Reset the animation for it to be resumed with the correct parameters
            anim = Animation(1.0f, m_ratio, m_duration, m_func);
          }
        }
        break;
      }
    }
  }

void AnimatedApp::render(){
  INSTRUMENTATE(m_parent_ui)
    anim.Update();
    m_computeAnimation();
  
    const float scale_fac = anim.getProgress();
    const Texture drawing_image = scale(*m_showing, scale_fac);
    m_s_height = drawing_image.height;
    m_s_width = drawing_image.width;
    const Point drawing_pos = getConstraintedPos();

    
    if(drawing_image.data.colorspace == PixelType::Mono){
      m_parent_ui->buffer->drawBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.mono, drawing_image.width, drawing_image.height, m_mono_color);
    }
    else{
      m_parent_ui->buffer->drawRGBBitmap(drawing_pos.x, drawing_pos.y, drawing_image.data.rgb565, drawing_image.width, drawing_image.height);
    }
  }

//--------------------Checkbox CLASS---------------------------------------------------------------//

  void Checkbox::m_drawCheckboxOutline() const {
    INSTRUMENTATE(m_parent_ui)
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
    INSTRUMENTATE(m_parent_ui)
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

  Scene::Scene(std::initializer_list<UIElement*> elementGroup, UIElement* first_focus){

    for(const auto elem : elementGroup){
      elements.insert({elem->getId(),elem});
    }

    primaryElementID = first_focus ? first_focus->getId() : "";
  }

  
  void Scene::renderScene() const {
      if(!settings.scriptOnTop)
        m_script();

      for (const auto&[id, element] : elements)
      {
        if(element->draw){
          element->render();
          if(element->isFocused()&&element->focus_style==FocusStyle::Outline)
            element->drawFocusOutline(settings.focus.outline);
        }
      }

      if(settings.scriptOnTop)
        m_script();
    }

  UIElement* Scene::getElementByUUID(std::string UUID) const {
    if(!UUID.empty())
      return elements.at(UUID);
    else
      return nullptr;
  }

  void Scene::addParents(std::initializer_list<Scene*> scenes){
    for(const auto scene : scenes){
      parents.push_back(scene);
    }
  }

//--------------------UI CLASS---------------------------------------------------------------//

  UI::UI(Scene* first_scene, GFXcanvas16* framebuffer)
  {
    if (first_scene && framebuffer){
      buffer = framebuffer;
      focus = Focus(first_scene->primaryElementID);
      AddScene(first_scene);
      focus.focusScene(first_scene);
    }
  }

  void UI::AddScene(Scene* scene){

    scenes.push_back(scene);                 //KEEP IN MIND "REALLOCATES"
    scene->m_parent_ui = this;
    for(const auto&[id, element] : scene->elements){
      element->setUiListener(this);
    }
    
  }

  void UI::FocusScene(Scene* scene){
      focus.focusScene(scene);
    }

  void UI::Back(){
    if (focus.previousScene){
      Serial.println("Back!");
      if ( !(focus.activeScene->parents.empty()) ) {
        focus.focusScene(focus.previousScene);
      }
    }else{
      return;
    }
  }

  void UI::Click(){
    UIElement* focused = getFocused();
    if(focused)
      focused->click();
    else
      return;
  }


  /// @brief Focus the closest object in any direction
  /// @param direction The direction in counter clockwise degrees, with its origin being the center of the currently focused element (Right is 0)
  /// @param alg The focusing algorithm that you want to use (FocusingAlgorithm::Linear, FocusingAlgorithm::Cone)
  void UI::FocusDirection(unsigned int direction){
    INSTRUMENTATE(this)
    m_focusDir(direction);
  }

  /// @brief Focus the closest object in any direction
  /// @param direction The direction in counter clockwise degrees, with its origin being the center of the currently focused element (Right is 0)
  /// @param alg The focusing algorithm that you want to use (FocusingAlgorithm::Linear, FocusingAlgorithm::Cone)
  void UI::FocusDirection(Direction direction){
    INSTRUMENTATE(this)
    m_focusDir(static_cast<unsigned int>(direction));
  }

  void UI::Render(){
    if (focus.activeScene)
      focus.activeScene->renderScene();
    m_updateFocus();
  }

  void UI::m_focusDir(unsigned int direction){
    if(isFocusingFree()){
      m_focusing_busy = true;

      UIElement* next_element;
      if (focus.activeScene->elements.empty())
        return;
      else
        if (!focus.focusedElementID.empty())
          next_element = UiUtils::SignedDistance(direction, focus.activeScene, focus.activeScene->getElementByUUID(focus.focusedElementID));
        else
          next_element = UiUtils::SignedDistance(direction, focus.activeScene, nullptr);

      if (next_element){
        focus.focus(next_element->getId());
        focus.focusedElementID = next_element->getId();
      }
    }
  }

  void UI::m_updateFocus(){
    focus.update();
    m_focusing_busy = false;
  }

  #if PERFORMANCE_PROFILING
  void UI::printPerfStats(){
    for(const auto& [name, time] : m_perfValues){
      Serial.printf("%s = %dus\n", name.c_str(), time);
    }
  }

  void UI::m_addPerf(std::string key, uint32_t time){
    uint32_t tempVal = m_perfValues[key];
    if(time > tempVal){
      m_perfValues[key] = time;
    }
  }
  #endif

//--------------------UiUtils NAMESPACE---------------------------------------------------------------//

  namespace UiUtils{
    bool isPointInElement(Point point, UIElement* element){
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
    const Point centerPos(int x_pos, int y_pos, const unsigned int w, const unsigned int h){
        int new_x = round(static_cast<float>(x_pos)+(w*0.5f));
        int new_y = round(static_cast<float>(y_pos)+(h*0.5f));
        return Point(new_x, new_y);
      }


    UIElement* SignedDistance(const unsigned int direction, Scene* scene, UIElement* focused){
      Scene::SceneSettings::FocusingSettings& settings = scene->settings.focus;
      INSTRUMENTATE(focused->getParentUI())
      if (scene->elements.empty()){
        return nullptr;
      }

      if (settings.algorithm == FocusingAlgorithm::Linear)
      {
        Ray ray{settings.max_distance, 1, direction};
        switch (settings.accuracy){
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

        if (focused)
          return findElementInRay(focused, scene, ray);
        else{
          UIElement temp(10, 10, {static_cast<int>(scene->m_parent_ui->buffer->width()*0.5), static_cast<int>(scene->m_parent_ui->buffer->height()*0.5)}, true);
          UIElement* result = findElementInRay(&temp, scene, ray);
          return result;
        }
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
        if (focused)
          return findElementInCone(focused, scene, cone);
        else{
          UIElement temp(10, 10, {static_cast<int>(scene->m_parent_ui->buffer->width()*0.5), static_cast<int>(scene->m_parent_ui->buffer->height()*0.5)}, true);
          UIElement* result = findElementInCone(&temp, scene, cone);
          return result;
        }
      }

      return nullptr;
    }

    Point polarToCartesian(const float radius, const float angle){
      return Point(radius * cos(-angle * degToRadCoefficient), //x
                  radius * sin(-angle * degToRadCoefficient));//y
    }
    std::set<Point> computeConePoints(Point vertex, Cone cone){
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

    UIElement* findElementInCone(UIElement* focused, Scene* currentScene, const Cone& cone){
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
          tempPoint = polarToCartesian(b, i);
          tempPoint += centerPoint;
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

    UIElement* findElementInRay(UIElement* focused, Scene* currentScene, const Ray& ray){
      focused->focusable = false;
      const Point centerPoint = focused->getCenterPoint();
      Point tempPoint;
      
      for(int i = 0; i<ray.ray_length; i+=ray.step){
        tempPoint = polarToCartesian(i, ray.direction);
        tempPoint += centerPoint;
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

    const std::string constraintToString(const Constraint constraint){
      switch (constraint){
        case Constraint::TopLeft:     return "TopLeft";
        case Constraint::Top:         return "Top";
        case Constraint::TopRight:    return "TopRight";
        case Constraint::Left:        return "Left";
        case Constraint::Center:      return "Center";
        case Constraint::Right:       return "Right";
        case Constraint::BottomLeft:  return "BottomLeft";
        case Constraint::Bottom:      return "Bottom";
        case Constraint::BottomRight: return "BottomRight";
      }
      return "Error";
    }
  };
};