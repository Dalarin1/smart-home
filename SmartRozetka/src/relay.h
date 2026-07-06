#pragma once

#include <Arduino.h>
#include "globalFSM.h"

class Relay {
public:
    Relay(GlobalFSM& fsm);

    void begin();
    void actualize();

private:
    GlobalFSM& fsm;
};