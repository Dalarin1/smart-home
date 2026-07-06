#ifndef OLEG_HOME_DEVICE
#define OLEG_HOME_DEVICE
#include <Arduino.h>
#include <array>
#include <cstdint>

using MacAddress = std::array<uint8_t, 6>;

MacAddress toMacArray(const uint8_t* raw);
enum class DeviceType : uint8_t {
    SOCKET,
    LIGHT_SWITCH,
    SENSOR
};

enum class DeviceStatus : uint8_t {
    ONLINE,
    OFFLINE
};

struct DeviceInfo {
    MacAddress mac;
    DeviceType type;
    String name; 
    DeviceStatus status;
    uint32_t lastSeen;
    bool lastKnownState;
};

struct StoredDevice {
    MacAddress mac;
    DeviceType type;
    char name[32];
    bool lastKnownState;
};

#endif
