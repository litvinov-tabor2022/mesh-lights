#ifndef MESH_LIGHTS_NFCMANAGER_H
#define MESH_LIGHTS_NFCMANAGER_H

#include <memory>
#include <utility>
#include <optional>
#include "AbsDeviceManager.h"
#include <painlessMesh.h>
#include "PortalFramework.h"

class NfcManager : public AbsDeviceManager {
public:
    NfcManager(String key, Scheduler *scheduler) : keyString(std::move(key)), scheduler(scheduler) {}

    bool init() override {
        Serial.println("Initializing NFC manager...");
        std::optional<std::string> frameworkInitMessage = framework.begin(false, false);
        if (!frameworkInitMessage->empty()) {
            Debug.printf("Could not initialize framework! Err: %s\n", frameworkInitMessage.value().c_str());
            return false;
        }

        framework.addOnConnectCallback([this](PlayerData playerData, bool isReload) {
            if (!isReload) {
                this->turnOn();
                Debug.printf("Connected: player ID %d\n", playerData.user_id);
            } else {
                Debug.println("Tag successfully reloaded");
            }
            this->turnOn();
        });

        return true;
    }

    String key() override {
        return keyString;
    }

private:
    String keyString;
    Scheduler *scheduler;
    PortalFramework framework;
};


#endif //MESH_LIGHTS_NFCMANAGER_H
