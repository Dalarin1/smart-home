#ifndef OLEG_HOME_PROTOCOL
#define OLEG_HOME_PROTOCOL

#include <cstdint>

constexpr const char* PAIRING_SERVICE_UUID = "3c8066dd-23f7-450d-a445-2ea1375c8e93";
constexpr const char* CLAIM_CHAR_UUID = "d474fd17-698e-45c7-bd5f-9d8745fde323";
constexpr const char* INFO_CHAR_UUID = "255cc188-35c4-4fad-a4c9-0301b9e3b422";

constexpr const char* HUB_AP_SSID = "Oleg-WiFi";
constexpr const char* HUB_AP_PASSWORD = "qwertyuip";

constexpr const uint8_t WIFI_CHANNEL = 6;
constexpr const uint32_t BLE_SCAN_TIME_MS = 5000;
constexpr const uint16_t BLE_SCAN_INTERVAL_MS = 100;
constexpr const uint16_t BLE_SCAN_WINDOW_MS = 99;

constexpr const char* PREFS_NAMESPACE = "hub-devices";
constexpr const char* KEY_COUNT = "count";
constexpr const char* KEY_BLOB  = "list";

enum class Answer : uint8_t {
    YES = 0xFF,
    NO = 0x00,
    ERROR = 0xEE
};

namespace Commands {
    enum class Socket : uint8_t {
        FORGET = 0,
        OFF = 1,
        ON = 2,
        TOGGLE = 4,
        GET_STATUS = 8
    };
    using LightSwitch = Socket;
    // место под другие устройства
};

#endif