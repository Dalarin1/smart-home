#include "globalFSM.h"

GlobalFSM::GlobalFSM(DeviceState &newDeviceState) {
  deviceState = newDeviceState;
  air.setFsm(this);
}

void GlobalFSM::setState(GlobalState newState) {
  deviceState.globalState = newState;
}

GlobalState GlobalFSM::getState() const {
  return deviceState.globalState;
}

void GlobalFSM::setAction(Action newAction) {
  deviceState.action = newAction;
}

bool GlobalFSM::getRelay() const {
  return deviceState.relay;
}

void GlobalFSM::setReseting(bool state) {
  deviceState.reseting = state;
}

bool GlobalFSM::getReseting() const {
  return deviceState.reseting;
}

bool GlobalFSM::getAdvertising() const {
  return deviceState.advertising;
}

void GlobalFSM::setPrefs(Preferences* newPrefs) {
  prefs = newPrefs;
  air.setPrefernces(prefs);
}

Preferences* GlobalFSM::getPrefs() {
  return prefs;
}

void GlobalFSM::update() {
  switch (deviceState.action) {
    case TOGGLE:
      deviceState.relay = !deviceState.relay;
      break;

    case ON:
      deviceState.relay = true;
      break;

    case OFF:
      deviceState.relay = false;
      break;

    case RESET:
      deviceState.globalState = RESETTING;
      break;

    case NONE:
      break;

    default:
      break;
  }
  deviceState.action = NONE;

  bool enteredState = (deviceState.globalState != lastLoggedState);
  lastLoggedState = deviceState.globalState;

  switch (deviceState.globalState) {
    case BOOT:
      if (enteredState) Serial.println("BOOT");
      air.debugPrefs();
      if(air.tryFindMacAddr()){
        air.initEspNow();
        setState(PAIRED);
      } else {
        setState(UNPAIRED);
      }
      break;

    case UNPAIRED:
      if (!deviceState.advertising && !air.hasGivenUp()) {
          air.startAdvertising([&](){ deviceState.advertising = false; setState(BOOT); });
          deviceState.advertising = true;
          Serial.println("UNPAIRED: searching for hub");
      } else if (deviceState.advertising && air.advertisingTimedOut()) {
          air.stopAdvertising();
          deviceState.advertising = false;
          Serial.println("UNPAIRED: standalone mode");
      }
      break;

    case PAIRED:
      if (enteredState) Serial.println("PAIRED");
      air.update();
      break;

    case RESETTING:
      if (enteredState) Serial.println("RESETTING");
      if (!deviceState.reseting) {
        deviceState.reseting = true;
        timeStRST = millis();
        air.forgetHub();
      }
      if (millis() - timeStRST >= RTSTime) {
        ESP.restart();
      }
      break;
  }
}