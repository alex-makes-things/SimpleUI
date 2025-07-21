#include <Arduino.h>
#include <vector>

//Simple pushbutton wrapper
class Button{
  public:
  uint32_t m_last_update;
  const uint8_t pin;
  bool state;
  bool prevState;
  bool clickedOnce;
  Button(const uint8_t gpio):pin(gpio),state(false),prevState(false),clickedOnce(false), m_last_update(micros()){}
  void setup() const;
  void updateState();
  void remember(){prevState = state; clickedOnce=false;}
  private:
};

namespace ButtonUtils{
  uint32_t getMostRecentUpdate(const std::vector<Button*>& myButtons);
  void updateButtons(const std::vector<Button*>& myButtons);
  void rememberButtons(const std::vector<Button*>& myButtons);
  void setupButtons(const std::vector<Button*>& myButtons);
}

