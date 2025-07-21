#include "HardwareAid.h"

//--------------------Button CLASS---------------------------------------------------------------//

void Button::updateState()
{
    state = digitalRead(pin);
    if (micros() - m_last_update >= 2500UL) //Short delay of 5ms to prevent weird things from happening
    {
        if (state && (state != prevState))
        {
            clickedOnce = true;
        }
        else{
            clickedOnce = false;
        }
        m_last_update = micros();
    }
}



void Button::setup() const {
    pinMode(pin, INPUT);
  }


//--------------------ButtonUtils NAMESPACE---------------------------------------------------------------//

void ButtonUtils::updateButtons(const std::vector<Button*>& myButtons) {
    if(micros()-getMostRecentUpdate(myButtons) >= 5000){
        for (Button* btn : myButtons) {
            btn->updateState();
        }
    }
}

void ButtonUtils::rememberButtons(const std::vector<Button*>& myButtons) {
    for (Button* btn : myButtons) {
        btn->remember();
    }
}

void ButtonUtils::setupButtons(const std::vector<Button*>& myButtons) {
    for (Button* btn : myButtons) {
        btn->setup();
    }
}

uint32_t ButtonUtils::getMostRecentUpdate(const std::vector<Button*>& myButtons){
  Button* mostRecent = *std::max_element(myButtons.begin(), myButtons.end(),
        [](const Button* a, const Button* b) {return a->m_last_update < a->m_last_update;});
  return mostRecent->m_last_update;
  }