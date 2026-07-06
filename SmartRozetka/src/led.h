#pragma once

#include <Arduino.h>
#include "globalFSM.h"

class Led {
public:
    Led(GlobalFSM& fsm);

    void begin();
    void update();

private:
    GlobalFSM& fsm;
};