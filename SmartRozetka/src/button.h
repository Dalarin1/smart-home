#pragma once

#include <Arduino.h>
#include "globalFSM.h"

class Button {
public:
    Button(GlobalFSM& fsm);

    void begin();
    void update();

private:
    GlobalFSM& fsm;

    bool lastReading = HIGH;
    bool state = HIGH;

    unsigned long debounceTime = 0;
    unsigned long pressTime = 0;
    bool longPress = false;
};