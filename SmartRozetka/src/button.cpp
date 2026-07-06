#include "button.h"
#include "settings.h"

static constexpr uint32_t DEBOUNCE_MS = 30;
static constexpr uint32_t LONG_PRESS_MS = 3000;

Button::Button(GlobalFSM& fsm)
    : fsm(fsm)
{
}

void Button::begin() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void Button::update() {
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != lastReading)
        debounceTime = millis();

    if (millis() - debounceTime > DEBOUNCE_MS) {

        if (reading != state) {
            state = reading;

            if (state == LOW) {
                pressTime = millis();
                longPress = false;
            } else if (!longPress) {
                fsm.setAction(TOGGLE);
            }
        }

        if (state == LOW &&
            !longPress &&
            millis() - pressTime >= LONG_PRESS_MS) {

            longPress = true;
            fsm.setAction(RESET);
        }
    }

    lastReading = reading;
}