#ifndef MESH_LIGHTS_LIGHTMANAGER_H
#define MESH_LIGHTS_LIGHTMANAGER_H

#include <memory>
#include <utility>
#include "AbsDeviceManager.h"
#include <painlessMesh.h>

#define LIGHT_DELAY TASK_SECOND * 2.5

class LightManager : public AbsDeviceManager {
public:
    explicit LightManager(int outputPin) : outputPin(outputPin) {}

    LightManager(int outputPin, String key, Scheduler *scheduler) : outputPin(outputPin), keyString(std::move(key)),
                                                                    scheduler(scheduler) {
        initTimeout(scheduler);
    }

    void init() {
        Serial.println("Initializing Light manager...");
        this->setOnToggleCallback([this](bool state) {
            if (state) {
                turnOnLight();
            } else {
                turnOffLight();
            }
        });
        this->setOnTurnOnCallback([this]() {
            turnOnLight();
        });
        this->setOnTurnOffCallback([this]() {
            turnOffLight();
        });
        this->setOnTimeout([this]() {
            timeoutHandler();
        });
    }

    String key() override {
        return keyString;
    }

//    void loop() const {
//        if (millis() - lightSince > LIGHT_DELAY) {
//            digitalWrite(outputPin, LOW);
//        }
//    }

private:
    void turnOnLight() {
        Serial.println("Light is ON");
        digitalWrite(outputPin, HIGH);
    }

    void turnOffLight() {
        Serial.println("Light is OFF");
        digitalWrite(outputPin, LOW);
    }

    void timeoutHandler() {
        Serial.println("Timeout");
        turnOffLight();
    }

    unsigned long lightSince = 0;
    int outputPin;
    String keyString;
    Scheduler *scheduler;
};


#endif //MESH_LIGHTS_LIGHTMANAGER_H
