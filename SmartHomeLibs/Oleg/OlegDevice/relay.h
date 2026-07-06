#ifndef OLEG_HOME_RELAY
#define OLEG_HOME_RELAY

#include <Arduino.h>
#include <cstdint>

class Relay {
private:
    static void apply();

public:
    static bool is_on;
    static uint8_t pin;

    static void begin(uint8_t pin);

    static void turnOn();
    static void turnOff();

    static void toggle();

    static bool isOn();
};

#endif