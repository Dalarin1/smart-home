#ifndef OLEG_HOME_CLAIM_CODEC
#define OLEG_HOME_CLAIM_CODEC

#include <Arduino.h>
#include <cstdint>

struct ClaimPayload {
    uint8_t hub_mac[6] = {0};
    uint8_t channel;
    String token;

    static ClaimPayload encode(const String& mac, uint8_t channel, const String& token);
    static ClaimPayload decode(uint8_t* bytes, uint16_t len);
    uint16_t size() const;
};

#endif