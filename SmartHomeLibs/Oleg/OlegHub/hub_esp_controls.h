#ifndef OLEG_HOME_HUB_ESP_CONTROLS
#define OLEG_HOME_HUB_ESP_CONTROLS

#include "protocol.h"
#include "device.h"
#include <cstdint>
#include <map>
#include <array>
#include <Arduino.h>
#include <Preferences.h>
#include <vector>

class HubEspNow {
public:
    static std::map<MacAddress, DeviceInfo> connected_devices; 

    static bool init();
    static bool add_peer(const MacAddress& mac, DeviceType type, String name);
    static void remove_peer(const MacAddress& mac);
    static bool send_command_to(const MacAddress& mac, Commands::Socket command);
    static void load_devices();
    static void save_devices();

private:
    static Preferences prefs;
    static bool register_esp_now_peer(const MacAddress& mac); 
};

#endif
