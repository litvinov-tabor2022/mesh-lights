#ifndef ESPNOW_POC_STATEMANAGER_H
#define ESPNOW_POC_STATEMANAGER_H


#include <map>
#include <memory>
#include "Arduino.h"
#include "devices/AbsDeviceManager.h"
#include "ArduinoJson.h"
#include "devices/Action.h"

class StateManager {
public:
    void registerDeviceManager(const std::shared_ptr<AbsDeviceManager> &deviceManager) {
        devicesManagers.insert({deviceManager->key(), deviceManager});
    }

    bool processAction(const Action& action) {
        auto device = devicesManagers.find(action.device);
        if (device != devicesManagers.end()) {
            if (action.state) {
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
        JsonArray devices = doc.createNestedArray("devices");
        for (const auto& device: devicesManagers) {
            devices.add(device.second->getAction().serialize());
        }
        return doc;
    }

private:
    std::map<String, std::shared_ptr<AbsDeviceManager>> devicesManagers;
};


#endif //ESPNOW_POC_STATEMANAGER_H
