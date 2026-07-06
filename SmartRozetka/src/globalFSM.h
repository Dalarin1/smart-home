#pragma once

#include <NimBLEDevice.h>
#include <Preferences.h>
#include <Arduino.h>
#include "air.h"
#include "settings.h"
#include "globalState.h"
#include "deviceState.h"

class GlobalFSM {
public:
    GlobalFSM(DeviceState &newDeviceState);

    void update();
    void setState(GlobalState newState);
    GlobalState getState() const;

    void setAction(Action newAction);
    bool getRelay() const;

    void setReseting(bool state);
    bool getReseting() const;

    void setPrefs(Preferences* newPrefs);
    Preferences* getPrefs();

    bool getAdvertising() const;

    void setAir(Air newAir);
private:
    
    DeviceState deviceState;
    GlobalState lastLoggedState = static_cast<GlobalState>(-1);
    unsigned long timeStRST = 0;
    Preferences* prefs;
    Air air;
};