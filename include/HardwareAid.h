#include <Arduino.h>
#include <vector>

class Button{
  public:
  uint8_t pin;
  bool state;
  bool prevState;
  bool clickedOnce;
  Button(uint8_t gpio):pin(gpio){}
  void setup(){
    pinMode(pin, INPUT);
  }
  void updateState(){
    state = digitalRead(pin);
    if (state && (state != prevState)){
      clickedOnce = true;
    }else{clickedOnce = false;}
  }
  void remember(){
    prevState = state;
  }
};

void updateButtons(const std::vector<Button*>& myButtons) {
    for (Button* btn : myButtons) {
        btn->updateState();
    }
}

void rememberButtons(const std::vector<Button*>& myButtons) {
    for (Button* btn : myButtons) {
        btn->remember();
    }
}