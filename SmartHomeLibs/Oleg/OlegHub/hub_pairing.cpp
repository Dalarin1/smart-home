#include "hub_pairing.h"
#include "device.h"
#include "claim_codec.h"
#include "protocol.h"
#include "hub_esp_controls.h"
#include <WiFi.h>

std::map<std::string, NimBLEAddress> HubPairing::pending_devices;
bool HubPairing::scan_in_progress;
bool HubPairing::scan_finished;

class ClientCallbacks : public NimBLEClientCallbacks {
    void onDisconnect(NimBLEClient* pClient, int reason) override {
        Serial.printf("[claim] клиент отключился, reason=%d\n", reason);
    }
    void onConnect(NimBLEClient* pClient) override {
        Serial.println("[claim] onConnect callback сработал");
    }
};

static ClientCallbacks clientCallbacks;

void HubPairing::ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
    scan_in_progress = false;
    scan_finished = true;
}

void HubPairing::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* adverised_device) {
    if (adverised_device->isAdvertisingService(NimBLEUUID(PAIRING_SERVICE_UUID))) {
        NimBLEAddress addr = adverised_device->getAddress();
        pending_devices[addr.toString()] = addr;
    }
}

void HubPairing::init(const char* device_name) {
    NimBLEDevice::init(device_name);

    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new ScanCallbacks());
    pScan->setActiveScan(true);
    pScan->setInterval(BLE_SCAN_INTERVAL_MS);
    pScan->setWindow(BLE_SCAN_WINDOW_MS);
}

bool HubPairing::scan() {
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pending_devices.clear();
    scan_in_progress = true;
    scan_finished = false;
    return pScan->start(BLE_SCAN_TIME_MS);
}

bool HubPairing::claim(NimBLEAddress addr, MacAddress& out_mac) {
    NimBLEDevice::getScan()->stop();
    delay(100);

    NimBLEClient* pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(&clientCallbacks, false);
    pClient->setConnectTimeout(5000);

    NimBLERemoteService* service = nullptr;
    NimBLERemoteCharacteristic* info_char = nullptr;
    NimBLERemoteCharacteristic* claim_char = nullptr;
    std::string mac_str;
    ClaimPayload payload;
    bool writeOk = false;

    if (!pClient->connect(addr)) {
        Serial.println("[claim] connect() failed");
        goto on_error;
    }

    service = pClient->getService(NimBLEUUID(PAIRING_SERVICE_UUID));
    if (!service) {
        Serial.println("[claim] pairing service not found among discovered services");
        goto on_error;
    }

    info_char = service->getCharacteristic(INFO_CHAR_UUID);
    if (!info_char || !info_char->canRead()) {
        Serial.println("[claim] info characteristic missing or not readable");
        goto on_error;
    }

    claim_char = service->getCharacteristic(CLAIM_CHAR_UUID);
    if (!claim_char || !claim_char->canWrite()) {
        Serial.println("[claim] claim characteristic missing or not writable");
        goto on_error;
    }

    mac_str = info_char->readValue();
    if (mac_str.size() < 6) {
        Serial.printf("[claim] info characteristic returned %d bytes, expected 6\n", mac_str.size());
        goto on_error;
    }
    for (byte i = 0; i < 6; i++) {
        out_mac[i] = mac_str[i];
    }

    payload = ClaimPayload::encode(WiFi.softAPmacAddress(), WIFI_CHANNEL, "oleg_token_228_super_secure");
    writeOk = claim_char->writeValue((const char*)&payload, payload.size(), false);
    if (!writeOk) {
        Serial.println("[claim] writeValue to claim characteristic failed");
    }

on_error:
    if (pClient->isConnected()) {
        pClient->disconnect();
    }
    NimBLEDevice::deleteClient(pClient);
    pending_devices.erase(addr);
    return writeOk;
}
