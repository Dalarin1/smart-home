#include "relay.h"
#include "settings.h"

Relay::Relay(GlobalFSM& fsm)
    : fsm(fsm)
{
}

void Relay::begin() {
    pinMode(RELAY_PIN, OUTPUT);
}

void Relay::actualize() {
    digitalWrite(RELAY_PIN, !fsm.getRelay());
}