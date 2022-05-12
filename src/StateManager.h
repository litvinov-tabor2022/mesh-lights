#ifndef ESPNOW_POC_STATEMANAGER_H
#define ESPNOW_POC_STATEMANAGER_H

#include <map>
#include <memory>
#include <utility>
#include "Arduino.h"
#include "devices/AbsDeviceManager.h"
#include "ArduinoJson.h"
#include "devices/Action.h"

class StateManager {
public:
    explicit StateManager(std::string deviceName) : deviceName(std::move(deviceName)) {}

    void registerDeviceManager(const std::shared_ptr<AbsDeviceManager> &deviceManager, const String &key) {
        devicesManagers.insert({key, deviceManager});
    }

    bool processAction(const Action &action) {
        auto device = devicesManagers.find(action.device);
        if (device != devicesManagers.end()) {
            if (action.state == Action::ON) {
                devicesManagers.find(action.device)->second->turnOn();
            } else {
                devicesManagers.find(action.device)->second->turnOff();
            }
            return true;
        } else {
            Serial.printf("Device %s not found\n", action.device.c_str());
        }
        return false;
    }

    DynamicJsonDocument serialize() {
        DynamicJsonDocument doc(1024);
        doc["name"] = deviceName;
        JsonArray devices = doc.createNestedArray("devices");
        for (const auto &device: devicesManagers) {
            devices.add(device.second->getAction().serialize());
        }
        return doc;
    }

private:
    std::map<String, std::shared_ptr<AbsDeviceManager>> devicesManagers;
    std::string deviceName;
};


#endif //ESPNOW_POC_STATEMANAGER_H
