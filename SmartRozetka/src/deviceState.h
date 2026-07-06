#pragma once

#include "globalState.h"

enum Action {
  NONE,
  TOGGLE,
  ON,
  OFF,
  RESET
};

class DeviceState {
public:
  DeviceState() : globalState(GlobalState::BOOT), relay(false), action(Action::NONE), reseting(false), advertising(false) {}

  GlobalState globalState;
  bool relay;
  Action action;
  bool reseting;
  bool advertising;
};