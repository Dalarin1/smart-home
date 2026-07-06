#include "led.h"
#include "settings.h"

Led::Led(GlobalFSM& fsm)
    : fsm(fsm) {
}

void Led::begin() {
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
}

void Led::update() {
    // pairing off
    if (fsm.getState() == PAIRED) {
        digitalWrite(LED_PIN1, HIGH);
        digitalWrite(LED_PIN2, HIGH);
    }
    // реле red
    digitalWrite(LED_PIN1, fsm.getRelay());

    // поиск green
    static bool blinkState = false;
    static unsigned long lastBlink = 0;

    if (millis() - lastBlink >= 500) {
        lastBlink = millis();
        blinkState = !blinkState;
    }

    if (fsm.getAdvertising()) {
        digitalWrite(LED_PIN2, blinkState);
    }

    // ресет yellow
    if (fsm.getState() == RESETTING) {
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(LED_PIN2, LOW);
    }
}