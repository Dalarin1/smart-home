#include "hub_esp_controls.h"

#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>
#include <vector>

std::map<MacAddress, DeviceInfo> HubEspNow::connected_devices;
Preferences HubEspNow::prefs;


bool HubEspNow::init() {
    return esp_now_init() == ESP_OK;
}

bool HubEspNow::register_esp_now_peer(const MacAddress& mac) {
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac.data(), 6);
    peer.channel = WIFI_CHANNEL;
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_AP;
    return esp_now_add_peer(&peer) == ESP_OK;
}

bool HubEspNow::add_peer(const MacAddress& mac, DeviceType type, String name) {
    if (connected_devices.count(mac)) {
        return true; // уже знаем это устройство
    }

    if (!register_esp_now_peer(mac)) {
        return false;
    }

    DeviceInfo info{};
    info.mac = mac;
    info.type = type;
    info.name = name;
    info.status = DeviceStatus::ONLINE;
    info.lastSeen = millis();
    info.lastKnownState = false;

    connected_devices[mac] = info;
    save_devices();
    return true;
}

void HubEspNow::remove_peer(const MacAddress& mac) {
    esp_now_del_peer(mac.data());
    connected_devices.erase(mac);
    save_devices();
}

bool HubEspNow::send_command_to(const MacAddress& mac, Commands::Socket command) {
    uint8_t cmd_binary = (uint8_t)(command);
    const uint8_t* raw_mac = mac.data();
    esp_err_t res = esp_now_send(raw_mac, &cmd_binary, 1);
    return res == ESP_OK;
}

void HubEspNow::save_devices() {
    std::vector<StoredDevice> buffer;
    buffer.reserve(connected_devices.size());

    for (auto& kv : connected_devices) {
        StoredDevice sd{};
        sd.mac = kv.second.mac;
        sd.type = kv.second.type;
        sd.lastKnownState = kv.second.lastKnownState;
        strncpy(sd.name, kv.second.name.c_str(), sizeof(sd.name) - 1);
        sd.name[sizeof(sd.name) - 1] = '\0';
        buffer.push_back(sd);
    }

    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putUInt(KEY_COUNT, buffer.size());
    if (!buffer.empty()) {
        prefs.putBytes(KEY_BLOB, buffer.data(), buffer.size() * sizeof(StoredDevice));
    } else {
        prefs.remove(KEY_BLOB);
    }
    prefs.end();
}

void HubEspNow::load_devices() {
    prefs.begin(PREFS_NAMESPACE, true);

    uint32_t count = prefs.getUInt(KEY_COUNT, 0);
    size_t expectedBytes = count * sizeof(StoredDevice);
    size_t actualBytes = prefs.getBytesLength(KEY_BLOB);

    if (count == 0 || actualBytes != expectedBytes) {
        prefs.end();
        return;
    }

    std::vector<StoredDevice> buffer(count);
    prefs.getBytes(KEY_BLOB, buffer.data(), expectedBytes);
    prefs.end();

    for (auto& sd : buffer) {
        if (!register_esp_now_peer(sd.mac)) {
            Serial.println("[hub] не удалось зарегистрировать peer из Preferences");
            continue;
        }

        DeviceInfo info{};
        info.mac = sd.mac;
        info.type = sd.type;
        info.name = String(sd.name);
        info.status = DeviceStatus::OFFLINE;
        info.lastSeen = 0;
        info.lastKnownState = sd.lastKnownState;

        connected_devices[sd.mac] = info;
    }

    Serial.printf("[hub] загружено устройств из Preferences: %d\n", (int)connected_devices.size());
}
