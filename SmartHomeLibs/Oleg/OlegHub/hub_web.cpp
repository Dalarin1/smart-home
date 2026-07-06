// hub_web.cpp
#include "hub_web.h"
#include "hub_esp_controls.h"
#include "hub_pairing.h"
#include "protocol.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

static AsyncWebServer server(80);

static String macToString(const MacAddress& mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

static const char* deviceTypeToString(DeviceType t) {
    switch (t) {
    case DeviceType::SOCKET:
        return "socket";
    case DeviceType::LIGHT_SWITCH:
        return "light_switch";
    case DeviceType::SENSOR:
        return "sensor";
    default:
        return "unknown";
    }
}

static void handleGetDevices(AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (auto& [mac, info] : HubEspNow::connected_devices) {
        JsonObject obj = arr.add<JsonObject>();
        obj["mac"] = macToString(mac);
        obj["name"] = info.name;
        obj["type"] = deviceTypeToString(info.type);
        obj["status"] = info.status == DeviceStatus::ONLINE ? "online" : "offline";
        obj["state"] = info.lastKnownState;
        obj["lastSeen"] = info.lastSeen;
    }

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
}

static void handleGetPending(AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (auto& addr : HubPairing::pending_devices) {
        JsonObject obj = arr.add<JsonObject>();
        obj["address"] = addr.second.toString();
    }

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
}

static void handleScanStart(AsyncWebServerRequest* request) {
    bool ok = HubPairing::scan();
    request->send(ok ? 200 : 500, "application/json",
                  ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

static void handleScanStop(AsyncWebServerRequest* request) {
    NimBLEDevice::getScan()->stop();
    request->send(200, "application/json", "{\"ok\":true}");
}

// POST /api/claim  body: {"address": "AA:BB..", "type": "socket", "name": "розетка"}
static void handleClaim(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    if(HubPairing::scan_in_progress){
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"wait a sec\"}");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, data, len) != DeserializationError::Ok) {
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
        return;
    }

    String addrStr = doc["address"] | "";
    String typeStr = doc["type"] | "socket";
    String name = doc["name"] | "Новое устройство";

    NimBLEAddress addr(std::string(addrStr.c_str(), addrStr.length()), 0);

    auto it = HubPairing::pending_devices.find(addrStr.c_str());
    if (it == HubPairing::pending_devices.end()) {
        request->send(404, "application/json", "{\"ok\":false,\"error\":\"device not in pending list\"}");
        return;
    }

    MacAddress deviceMac{};
    bool claimed = HubPairing::claim(it->second, deviceMac); 
    if (!claimed) {
        request->send(500, "application/json", "{\"ok\":false,\"error\":\"claim failed\"}");
        return;
    }

    DeviceType type = DeviceType::SOCKET; 
    bool added = HubEspNow::add_peer(deviceMac, type, name);

    request->send(added ? 200 : 500, "application/json",
                  added ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"peer add failed\"}");
}

// POST /api/command  body: {"mac": "AA:BB:...", "command": "toggle"}
static void handleCommand(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    JsonDocument doc;
    if (deserializeJson(doc, data, len) != DeserializationError::Ok) {
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
        return;
    }

    String macStr = doc["mac"] | "";
    String cmdStr = doc["command"] | "";

    unsigned int v[6];
    if (sscanf(macStr.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
               &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) != 6) {
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad mac\"}");
        return;
    }
    MacAddress mac;
    for (int i = 0; i < 6; i++)
        mac[i] = (uint8_t)v[i];

    Commands::Socket cmd;
    if (cmdStr == "on")
        cmd = Commands::Socket::ON;
    else if (cmdStr == "off")
        cmd = Commands::Socket::OFF;
    else if (cmdStr == "toggle")
        cmd = Commands::Socket::TOGGLE;
    else if (cmdStr == "status")
        cmd = Commands::Socket::GET_STATUS;
    else if (cmdStr == "disconnect")
        cmd = Commands::Socket::FORGET;
    else {
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"unknown command\"}");
        return;
    }

    bool ok = HubEspNow::send_command_to(mac, cmd);
    request->send(ok ? 200 : 500, "application/json",
                  ok ? "{\"ok\":true}" : "{\"ok\":false}");

    if (cmd == Commands::Socket::FORGET){
        HubEspNow::remove_peer(mac);
    }
}

void HubWebService::init() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed!");
        return;
    }

    server.on("/api/devices", HTTP_GET, handleGetDevices);
    server.on("/api/pending", HTTP_GET, handleGetPending);
    server.on("/api/scan/start", HTTP_POST, handleScanStart);
    server.on("/api/scan/stop", HTTP_POST, handleScanStop);

    server.on("/api/claim", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) { handleClaim(request, data, len); });

    server.on("/api/command", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) { handleCommand(request, data, len); });

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
    Serial.println("Веб-сервер запущен");
}
