#include "air.h"
#include "globalFSM.h"

bool Air::has_command;
Command Air::command;

void Air::setPrefernces(Preferences* newPrefs){
  prefs = newPrefs;
}

void Air::debugPrefs()
{
    bool connected = prefs->getBool("conn_hub", false);

    Serial.print("conn_hub: ");
    Serial.println(connected ? "true" : "false");

    uint8_t mac[6];
    size_t len = prefs->getBytes("hub_mac", mac, 6);

    Serial.print("hub_mac length: ");
    Serial.println(len);

    if (len == 6) {
        Serial.printf("hub_mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      mac[0], mac[1], mac[2],
                      mac[3], mac[4], mac[5]);
    }
}

bool Air::tryFindMacAddr(){
  know_hub = prefs->getBool("conn_hub", false);
  if(!know_hub){
    return false;
  }
  else {
    uint8_t v[6];
    size_t bytes = prefs->getBytes("hub_mac", v, 6);
    if(bytes != 6){
      know_hub = false;
      return false;
    }
    for(uint8_t i = 0; i < 6; i++){
      hub_mac[i] = v[i];
    }
    know_hub = true;
    return true;
  }
}

void Air::forgetHub(){
  know_hub = false;
  for(uint8_t i = 0; i < 6; i++){
    hub_mac[i] = 0x00;
  }
  prefs->putBool("conn_hub", false);
  prefs->putBytes("hub_mac", hub_mac, 6);
}

Air::ClaimPayload Air::ClaimPayload::encode(const String& mac, uint8_t channel, const String& token) {
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

Air::ClaimPayload Air::ClaimPayload::decode(uint8_t* bytes, uint16_t len) {
    ClaimPayload payload;
    if (bytes == nullptr || len < 7) {
        return payload;
    }
    memcpy(payload.hub_mac, bytes, 6);
    payload.channel = bytes[6];
    payload.token = String((const char*)bytes + 7, len - 7);
    return payload;
}

uint16_t Air::ClaimPayload::size() const {
    return 6 + 1 + token.length();
} 

void Air::DisconnectCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& conninfo, int reason)
{
    Serial.println("BLE DISCONNECTED");

    if(!air->deviceClaimed){
        Serial.println("NOT CLAIMED");
        air->forgetHub();
        NimBLEDevice::startAdvertising();
    } else {
        Serial.println("SAVING HUB");
        air->prefs->putBool("conn_hub", true);
        air->prefs->putBytes("hub_mac", air->hub_mac, 6);
        on_advertising_stop();
    }
}

void Air::ClaimCallback::onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& info){
  ClaimPayload packet = ClaimPayload::decode((uint8_t*)(pChar->getValue().data()), pChar->getValue().size());
  for (byte i = 0; i < 6; i++) {
      air->hub_mac[i] = packet.hub_mac[i];
  }
  air->deviceClaimed = true;
}

void Air::startAdvertising(std::function<void()> on_advertising_stop){
    search_gave_up = false;
    NimBLEDevice::init("OLEG_ROZETKA");

    server = NimBLEDevice::createServer();
    server->setCallbacks(new DisconnectCallbacks(this, on_advertising_stop));

    NimBLEService* pService = server->createService(PAIRING_SERVICE_UUID);

    NimBLECharacteristic* pChar_claim = pService->createCharacteristic(
        CLAIM_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pChar_claim->setCallbacks(new ClaimCallback(this));

    NimBLECharacteristic* pChar_info = pService->createCharacteristic(
        INFO_CHAR_UUID,
        NIMBLE_PROPERTY::READ);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    uint8_t myWifiMac[6];
    WiFi.macAddress(myWifiMac);

    pChar_info->setValue(myWifiMac, 6);

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(PAIRING_SERVICE_UUID);
    pAdvertising->start();

  advertising_start = millis();
}

void Air::stopAdvertising(){
    NimBLEDevice::getAdvertising()->stop();
    NimBLEDevice::deinit(true);
    search_gave_up = true;
    Serial.println("STANDALONE: timed out");
}

bool Air::advertisingTimedOut() const {
    return millis() - advertising_start >= ADVERTISING_TIME;
}

bool Air::hasGivenUp() const {
    return search_gave_up;
}

void Air::onEspNowRecv(const uint8_t* recv_info, const uint8_t* data, int data_len){
  if(data_len != 1){
    return;
  }
  has_command = true;
  command = (Command)(data[0]);
}

void Air::initEspNow(){
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setTxPower(WIFI_POWER_11dBm);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    // канал должен быть выставлен ДО esp_now_init, иначе peer уедет не туда
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW IS NOT WORKING RN");
        return;
    }
    
    if (esp_now_register_recv_cb(this->onEspNowRecv) != ESP_OK) {
        Serial.println("ESP-NOW CANT SET THE CALLBACK");
        return;
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, hub_mac, 6);
    peer.encrypt = false;
    peer.channel = WIFI_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println("Не смог добавить хаб как peer");
        return;
    }

    Serial.println("ESP-NOW настроен, ждём команды");
}
void Air::setFsm(GlobalFSM* new_fsm){
  fsm = new_fsm;
}
void Air::sendAnswer(Answer answer, uint8_t* mac = nullptr) {
    
    uint8_t byteAnswer = static_cast<uint8_t>(answer);
    esp_now_send(mac ? mac : hub_mac, &byteAnswer, 1);
}

void Air::update(){
  if(has_command){
    switch(command){
      case Command::TOGGLE:
        fsm->setAction(TOGGLE);
        sendAnswer(Answer::YES);
        break;
      case Command::ON:
        fsm->setAction(ON);
        sendAnswer(Answer::YES);
        break;
      case Command::OFF:
        fsm->setAction(OFF);
        sendAnswer(Answer::YES);
        break;
      case Command::GET_STATUS:
        sendAnswer(fsm->getRelay() ? Answer::YES : Answer::NO);
        break;
      case Command::FORGET:
        sendAnswer(Answer::YES);
        fsm->setState(RESETTING);
        break;
      default:
        sendAnswer(Answer::ERROR);
        break;
    }
    has_command = false;
  }
}