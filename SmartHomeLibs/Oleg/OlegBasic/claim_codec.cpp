#include "claim_codec.h"

ClaimPayload ClaimPayload::encode(const String& mac, uint8_t channel, const String& token) {
    ClaimPayload payload;
    unsigned int v[6];
    if (sscanf(mac.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
               &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            payload.hub_mac[i] = static_cast<uint8_t>(v[i]);
        }
    }
    payload.channel = channel;
    payload.token = token;

    return payload;
}

ClaimPayload ClaimPayload::decode(uint8_t* bytes, uint16_t len) {
    ClaimPayload payload;
    if (bytes == nullptr || len < 7) {
        return payload;
    }
    memcpy(payload.hub_mac, bytes, 6);
    payload.channel = bytes[6];
    payload.token = String((const char*)bytes + 7, len - 7);
    return payload;
}

uint16_t ClaimPayload::size() const {
    return 6 + 1 + token.length();
} 