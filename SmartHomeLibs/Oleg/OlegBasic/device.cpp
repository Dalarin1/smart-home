#include "device.h"


MacAddress toMacArray(const uint8_t* raw) {
    MacAddress mac;
    std::copy(raw, raw + 6, mac.begin());
    return mac;
}
