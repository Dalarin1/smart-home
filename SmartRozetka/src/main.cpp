#include <Arduino.h>
#include <Preferences.h>

#include "settings.h"
#include "deviceState.h"
#include "globalFSM.h"
#include "button.h"
#include "led.h"
#include "relay.h"

DeviceState deviceState;
GlobalFSM fsm(deviceState);
Preferences prefs;


Button button(fsm);
Led led(fsm);
Relay relay(fsm);

void setup() {
    Serial.begin(9600);
    pinMode(3, OUTPUT);
    pinMode(6, OUTPUT);
    digitalWrite(3, LOW);
    digitalWrite(6, HIGH);

    button.begin();
    led.begin();
    relay.begin();

    prefs.begin("rozetko", false);

    fsm.setPrefs(&prefs);
}

void loop() {
    button.update();
    fsm.update();
    led.update();
    relay.actualize();
}