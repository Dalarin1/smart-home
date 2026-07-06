#ifndef OLEG_HOME_HUB_PAIRING
#define OLEG_HOME_HUB_PAIRING
#include "device.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <map>

class HubPairing {
private:
    class ScanCallbacks : public NimBLEScanCallbacks {
        void onResult(const NimBLEAdvertisedDevice* advertised_device) override;
        void onScanEnd(const NimBLEScanResults& results, int reason) override;
    };

public:
    static std::map<std::string, NimBLEAddress> pending_devices;
    static bool scan_in_progress;
    static bool scan_finished;
    
    static void init(const char* device_name);
    static bool claim(NimBLEAddress address, MacAddress& out_mac);
    static bool scan();
};

#endif
