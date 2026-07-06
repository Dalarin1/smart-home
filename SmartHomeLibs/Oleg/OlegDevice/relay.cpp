#include "relay.h"

bool Relay::is_on;
uint8_t Relay::pin;

void Relay::apply() {
    digitalWrite(Relay::pin, Relay::is_on ? HIGH : LOW);
}


void Relay::begin(uint8_t pin) {
    Relay::pin = pin;
    pinMode(Relay::pin, OUTPUT);
    Relay::is_on = false;
    apply();
}

void Relay::turnOn() {
    if (!Relay::is_on) {
        Relay::is_on = true;
        apply();
    }
}

void Relay::turnOff() {
    if (Relay::is_on) {
        Relay::is_on = false;
        apply();
    }
}

void Relay::toggle() {
    Relay::is_on = !Relay::is_on;
    apply();
}

bool Relay::isOn() {
    return Relay::is_on;
}