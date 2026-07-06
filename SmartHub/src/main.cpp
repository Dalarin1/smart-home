#include "claim_codec.h"
#include "hub_esp_controls.h"
#include "hub_pairing.h"
#include "hub_web.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <NimBLEDevice.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// IP для локальной точки доступа
static const IPAddress LOCAL_IP(192, 168, 0, 113);
static const IPAddress GATEWAY_IP(192, 168, 0, 113);
static const IPAddress SUBNET_MASK(255, 255, 255, 0);

static String LOCAL_SSID;
static String LOCAL_PASS;

void RecvCallback(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
    MacAddress mac = toMacArray(mac_addr);
    auto it = HubEspNow::connected_devices.find(mac);
    if (it == HubEspNow::connected_devices.end()) {
        return; // ответ от незнакомого устройства — игнор или лог
    }

    it->second.lastSeen = millis();
    it->second.status = DeviceStatus::ONLINE;
    if (data_len == 1) {
        it->second.lastKnownState = (data[0] == static_cast<uint8_t>(Answer::YES));
    }

    Serial.printf("Получен ответ от (%s)[%02X:%02X:%02X:%02X:%02X:%02X] : 0x%02X \n",
                  it->second.name,
                  mac_addr[0], mac_addr[1], mac_addr[2],
                  mac_addr[3], mac_addr[4], mac_addr[5],
                  data_len > 0 ? data[0] : 0);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("HUB SETUP");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(LOCAL_IP, GATEWAY_IP, SUBNET_MASK);
    WiFi.softAP(HUB_AP_SSID, HUB_AP_PASSWORD, WIFI_CHANNEL);
    
    MDNS.begin("smarthome");
    MDNS.addService("http", "tcp", 80);

    HubEspNow::init();
    // пока не компилится
    // HubEspNow::set_recv_callback(RecvCallback);
    HubEspNow::load_devices();
    esp_now_register_recv_cb(RecvCallback);
    HubPairing::init("Oleg-Hub");
    HubWebService::init();
}

void loop() {
    if (HubPairing::scan_finished) {
        HubPairing::scan_finished = false;
        std::vector<NimBLEAddress> to_claim;
        to_claim.reserve(HubPairing::pending_devices.size());
        for (auto& kv : HubPairing::pending_devices) {
            to_claim.push_back(kv.second);
        }

        for (auto& addr : to_claim) {
            MacAddress out_mac{};
            if (HubPairing::claim(addr, out_mac)) {
                HubEspNow::add_peer(out_mac, DeviceType::SOCKET, "Rozetko");
            }
        }
    }
}
