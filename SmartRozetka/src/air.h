#pragma once
#include <cstdint>
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <WiFi.h>
#include <functional>
#include <esp_wifi.h>
#include <esp_now.h>


class GlobalFSM;

enum class Answer : uint8_t {
    YES = 0xFF,
    NO = 0x00,
    ERROR = 0xEE
};


enum class Command : uint8_t {
    FORGET = 0,
    OFF = 1,
    ON = 2,
    TOGGLE = 4,
    GET_STATUS = 8
};

class Air{
public:
  Air(){}
  bool tryFindMacAddr();
  void forgetHub();
  void debugPrefs();
  void setFsm(GlobalFSM* new_fsm);
  void setPrefernces(Preferences* newPrefs);
  void startAdvertising(std::function<void()> on_advertising_stop);
  void stopAdvertising();
  bool advertisingTimedOut() const;
  bool hasGivenUp() const;
  void initEspNow();
  static void onEspNowRecv(const uint8_t* recv_info, const uint8_t* data, int data_len);
  void sendAnswer(Answer answer, uint8_t* mac);
  void update();

private: 
  struct ClaimPayload {
      uint8_t hub_mac[6] = {0};
      uint8_t channel;
      String token;

      static ClaimPayload encode(const String& mac, uint8_t channel, const String& token);
      static ClaimPayload decode(uint8_t* bytes, uint16_t len);
      uint16_t size() const;
  };

  class DisconnectCallbacks : public NimBLEServerCallbacks {
  public:
    DisconnectCallbacks(Air* owner, std::function<void()> new_on_advertising_stop) : air(owner), on_advertising_stop(new_on_advertising_stop)  {}
      void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& conninfo, int reason) override;
  private:
    Air* air;
    std::function<void()> on_advertising_stop;
  };

  class ClaimCallback : public NimBLECharacteristicCallbacks {
  public:
    ClaimCallback(Air* owner) : air(owner) {}
      void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& info) override;

  private:
    Air* air;
  };
  GlobalFSM* fsm;
  Preferences* prefs;
  bool know_hub;
  uint8_t hub_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static bool has_command;
  static Command command;
  bool deviceClaimed;
  NimBLEServer* server;
  bool search_gave_up = false;
  unsigned long advertising_start = 0;
  const uint32_t ADVERTISING_TIME = 60000;
  const uint8_t WIFI_CHANNEL = 6;
  const char* PAIRING_SERVICE_UUID = "3c8066dd-23f7-450d-a445-2ea1375c8e93";
  const char* CLAIM_CHAR_UUID = "d474fd17-698e-45c7-bd5f-9d8745fde323";
  const char* INFO_CHAR_UUID = "255cc188-35c4-4fad-a4c9-0301b9e3b422";

};