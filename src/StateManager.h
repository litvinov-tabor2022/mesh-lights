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

    StateManager *registerDeviceManager(const std::shared_ptr<AbsDeviceManager> &deviceManager, const String &key) {
        auto actions = devicesManagers.find(key);
        if (actions != devicesManagers.end()) {
            actions->second.push_back(deviceManager);
        } else {
            devicesManagers.insert(std::pair<String, std::vector<std::shared_ptr<AbsDeviceManager>>>(key,
                                                                                                     std::vector<std::shared_ptr<AbsDeviceManager>>{
                                                                                                             deviceManager}));
        }
        return this;
    }

    bool processAction(const Action &action) {
        auto managers = devicesManagers.find(action.device);
        if (managers != devicesManagers.end()) {
            for (const auto &manager: managers->second) {
                Serial.println(manager->key());
                if (action.state == Action::STATE::ON) {
                    manager->turnOn();
                } else {
                    manager->turnOff();
                }
                if (action.timeout != Action::WITHOUT_TIMEOUT) manager->setTimeout(action.timeout);
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
        for (const auto &action: devicesManagers) {
            for (const auto &device: action.second) {
                Serial.printf("Action %s, device %s\n", action.first.c_str(), device->key().c_str());
                devices.add(device->getAction().serialize());
            }
        }
        return doc;
    }

    void print() {
        for (const auto &action: devicesManagers) {
            for (const auto &device: action.second) {
                Serial.printf("Action %s, device %s\n", action.first.c_str(), device->key().c_str());
            }
        }
    }


private:
    std::map<String, std::vector<std::shared_ptr<AbsDeviceManager>>> devicesManagers;
    std::string deviceName;
};


#endif //MESH_LIGHTS_STATEMANAGER_H
