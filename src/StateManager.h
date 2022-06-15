#ifndef MESH_LIGHTS_STATEMANAGER_H
#define MESH_LIGHTS_STATEMANAGER_H

#include <map>
#include <memory>
#include <utility>
#include "Arduino.h"
#include "devices/AbsDeviceManager.h"
#include "ArduinoJson.h"
#include "devices/Action.h"

class StateManager {
public:
    explicit StateManager(std::string deviceName) : deviceName(std::move(deviceName)) {
    }

    void registerDeviceManager(const std::shared_ptr<AbsDeviceManager> &deviceManager, const String &key) {
        devicesManagers.insert({key, deviceManager});
    }

    bool processAction(const Action &action) {
        auto devicePair = devicesManagers.find(action.device);
        if (devicePair != devicesManagers.end()) {
            auto device = devicePair->second;
            if (action.state == Action::ON) {
                device->turnOn();
            } else {
                device->turnOff();
            }
            if (action.timeout != Action::WITHOUT_TIMEOUT) device->setTimeout(action.timeout);
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


#endif //MESH_LIGHTS_STATEMANAGER_H
