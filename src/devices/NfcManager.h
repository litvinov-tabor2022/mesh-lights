#ifndef MESH_LIGHTS_NFCMANAGER_H
#define MESH_LIGHTS_NFCMANAGER_H

#include <memory>
#include <utility>
#include <optional>
#include "AbsDeviceManager.h"
#include <painlessMesh.h>
#include "PortalFramework.h"
#include "PlayerDataUtils.h"
#include "LedRingManager.h"

class NfcManager : public AbsDeviceManager {
public:
    NfcManager(String key, LedRingManager *ledRingManager, Scheduler *scheduler) : keyString(std::move(key)), scheduler(scheduler),
                                                                                   ledRingManager(ledRingManager) {}

    bool init() override {
        Serial.println("Initializing NFC manager...");
        std::optional<std::string> frameworkInitMessage = framework.begin(false, false);
        if (!frameworkInitMessage->empty()) {
            Debug.printf("Could not initialize framework! Err: %s\n", frameworkInitMessage.value().c_str());
            return false;
        }

        framework.addOnConnectCallback([this](PlayerData playerData, bool isReload) {
            if (!isReload) {
                Debug.printf("Connected: player ID %d\n", playerData.user_id);

                if (PlayerDataUtils::hasSkill(portal_Skill_Kouzlo_svetla, playerData)) {
                    this->playerData = playerData;
                    Serial.println("Player has required skill, sending the event");
                    this->turnOn();
                    this->ledRingManager->countdown(playerData.magic);
                } else {
                    Serial.println("Player has NOT required skill, ignoring");
                }
            } else {
                Debug.println("Tag successfully reloaded");
            }
        });

        framework.addOnDisconnectCallback([this] {
            this->turnOff();
        });

        return true;
    }

    PlayerData lastPlayerData(){
        return playerData;
    }

    String key() override {
        return keyString;
    }

private:
    String keyString;
    Scheduler *scheduler;
    LedRingManager *ledRingManager;
    PortalFramework framework;

    portal_PlayerData playerData;
};


#endif //MESH_LIGHTS_NFCMANAGER_H
