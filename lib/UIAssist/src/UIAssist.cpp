#include <UIAssist.h>


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


//--------------------FOCUS STRUCT---------------------------------------------------------------//
Focus::Focus(unsigned int sc, unsigned int ele){
    current.scene_id = sc;
    current.ele_id = ele;
  }

void Focus::focus(unsigned int sc, unsigned int ele){
    previous.scene_id = current.scene_id;
    previous.ele_id = current.ele_id;
    current.scene_id = sc;
    current.ele_id = ele;
  }

void Focus::update(){
    previous.scene_id = current.scene_id;
    previous.ele_id = current.ele_id;
    isFirstBoot = false;
  }

bool Focus::hasChanged(){
    return (previous.scene_id != current.scene_id || previous.ele_id != current.ele_id );
  }

bool Focus::isFocusing(Identity obj){
    return (current.ele_id == obj.ele_id && current.scene_id == obj.scene_id);
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

    //If the animation is active, the update logic runs
    if (enable){
      now = micros();
      elapsed = clamp(now-currentTime, 0, duration);

      //Here I add anything that has to run when the animation is in its done state
      if(isDone){
        //Breathing feature
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
        //Restarts the animation if the looping condition is met
        if(looping){
          resetAnim();
        }
      }

      //Interpolation
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

UIElement::UIElement(unsigned int w, unsigned int h, unsigned int posx, unsigned int posy){
    m_width = w;
    m_height = h;
    m_position = Coordinates(posx, posy);
  }

Coordinates UIElement::centerToCornerPos(unsigned int x_pos, unsigned int y_pos, unsigned int w, unsigned int h){
      unsigned int new_x = floor((float)x_pos-((float)w/2));
      unsigned int new_y = floor((float)y_pos-((float)h/2));
      return Coordinates(new_x, new_y);
    }


//--------------------UIElement CLASS---------------------------------------------------------------//
bool UIElement::isFocused(){
  return m_parent_ui->focusedElement==this;
}

//--------------------MonoImage CLASS---------------------------------------------------------------//

void MonoImage::setImg(Image8* img){
      m_body = img;
      m_width = img->width;
      m_height = img->height;
      m_color = img->color;
    }

void MonoImage::render(){
      anim.update();
      m_scale_fac = (overrideScaling) ? m_scale_fac : anim.getProgress();
      if (draw){
        Coordinates drawing_pos;
        if (m_scale_fac != 1){
          std::shared_ptr<Image8> scaled = scale(*m_body, m_scale_fac);
          drawing_pos = (centered) ? centerToCornerPos(m_position.x, m_position.y, scaled->width, scaled ->height) : m_position;
          canvas.drawBitmap(drawing_pos.x, drawing_pos.y, scaled->data, scaled->width, scaled->height, m_color);
        }else{
          drawing_pos = (centered) ? centerToCornerPos(m_position.x, m_position.y, m_body->width, m_body->height) : m_position;
          canvas.drawBitmap(drawing_pos.x, drawing_pos.y, m_body->data, m_body->width, m_body->height, m_color);
        }
      }
    }

//--------------------AnimatedMonoApp CLASS---------------------------------------------------------------//

void AnimatedMonoApp::handleAppSelectionAnimation(){
  if (m_parent_focus->hasChanged() || (m_parent_focus->isFirstBoot && m_parent_ui->focusedElement == this)){
    if(isFocused()){  
    //If the element is being focused
      if(isAnimating() && anim.getDirection()){
        anim.invert();
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
        anim.invert();
      }
    }
  }
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
  
}

void AnimatedMonoApp::render(){
  handleAppSelectionAnimation();
  anim.update();
  float scale_fac = anim.getProgress();
  if (draw)
  {
    Coordinates drawing_pos;
    if (scale_fac != 1)
    {
      std::shared_ptr<Image8> scaled = scale(*m_showing, scale_fac);
      drawing_pos = (centered) ? centerToCornerPos(m_position.x, m_position.y, scaled->width, scaled->height) : m_position;
      m_width = scaled->width;
      m_height = scaled->height;
      canvas.drawBitmap(drawing_pos.x, drawing_pos.y, scaled->data, scaled->width, scaled->height, m_color);
    }
    else
    {
      drawing_pos = (centered) ? centerToCornerPos(m_position.x, m_position.y, m_showing->width, m_showing->height) : m_position;
      m_width = m_showing->width;
      m_height = m_showing->height;
      canvas.drawBitmap(drawing_pos.x, drawing_pos.y, m_showing->data, m_showing->width, m_showing->height, m_color);
    }
  }
}


//--------------------Scene STRUCT---------------------------------------------------------------//

Scene::Scene(std::vector<UIElement*> elementGroup, unsigned int scene_id, bool active){
    for(int i = 0; i<elementGroup.size(); i++){
      elements.insert({elementGroup[i]->getId().ele_id,elementGroup[i]});
    }
    id = scene_id;
    isActive = active;
    for(UIElement* elem : elementGroup){
      elem->setSceneID(id);
    }
  }

void Scene::renderScene(){
    if(isActive){
      for(auto elem : elements){
        elem.second->render();
      }
    }
  }

//--------------------UI CLASS---------------------------------------------------------------//

UI::UI(UIElement* first_focus, Scene* first_scene){
    addScene(first_scene);
    focusScene(first_scene);
    focusedElement = first_focus;
    focus = Focus(first_scene->id, focusedElement->getId().ele_id);
  }

void UI::addScene(Scene* scene){
    scenes.insert({scene->id, scene});
    for(auto elem : scene->elements){
      elem.second->setFocusListener(&focus);
      elem.second->setUiListener(this);
    }
  }

void UI::focusScene(Scene* scene){
    focusedScene = scene;
    for(auto sc: scenes){
      sc.second->isActive=false;
    }
    focusedScene->isActive=true;
  }

void UI::focusDirection(unsigned int direction){
  if(!m_focusing_busy){
    m_focusing_busy = true;
    UIElement* next_element = UiUtils::isThereACollision(direction, focusedScene, focusedElement, max_focusing_distance);
    if(next_element){
        focus.focus(next_element->getId().scene_id, next_element->getId().ele_id);
        focusedElement = next_element;
    }
  }
}

void UI::update(){
  focus.update();
  m_focusing_busy = false;
}

//--------------------UiUtils NAMESPACE---------------------------------------------------------------//

bool UiUtils::areStill(std::vector<MonoImage*> images){
    int count = 0;
    for (MonoImage* image : images) {
      if(image->anim.getDone() || image->anim.getStart()){
        count++;
      }
    }
      return (count == images.size()) ? true : false;
  }

bool UiUtils::isPointInElement(Coordinates point, UIElement* element){
    Coordinates element_pos = element->getPos();
    if ((point.x >= element_pos.x && point.x <= element_pos.x + element->getWidth()) && (point.y >= element_pos.y && point.y <= element_pos.y + element->getHeight())){
      return true;
    }
    return false;
  }

Coordinates UiUtils::centerPos(int x_pos, int y_pos, unsigned int w, unsigned int h){
      int new_x = floor((float)x_pos+((float)w/2));
      int new_y = floor((float)y_pos+((float)h/2));
      return Coordinates(new_x, new_y);
  }

UIElement* UiUtils::isThereACollision(unsigned int direction, Scene* scene, UIElement* focused, unsigned int max_distance){

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