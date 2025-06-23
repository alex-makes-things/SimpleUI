#include <Arduino.h>
#include <vector>

//Simple pushbutton wrapper
class Button{
  public:
  uint64_t m_last_update;
  uint8_t pin;
  bool state;
  bool prevState;
  bool clickedOnce;
  Button(uint8_t gpio):pin(gpio){m_last_update=micros();}
  void setup();
  void updateState();
  void remember(){prevState = state;}
  private:
};

namespace ButtonUtils{
  uint64_t getMostRecentUpdate(const std::vector<Button*>& myButtons);
  void updateButtons(const std::vector<Button*>& myButtons);
  void rememberButtons(const std::vector<Button*>& myButtons);
  void setupButtons(const std::vector<Button*>& myButtons);
}

