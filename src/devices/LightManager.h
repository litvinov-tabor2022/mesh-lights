#ifndef ESPNOW_POC_LIGHTMANAGER_H
#define ESPNOW_POC_LIGHTMANAGER_H

#include <memory>
#include <utility>
#include "AbsDeviceManager.h"
#include <TaskScheduler.h>

class LightManager : public AbsDeviceManager {
public:
    explicit LightManager(int outputPin) : outputPin(outputPin) {}

    LightManager(int outputPin, String key) : outputPin(outputPin), keyString(std::move(key)) {}

    void init() {
        Serial.println("Initializing Light manager...");
        this->setOnToggleCallback([this](bool state) {
            if (state) {
                Serial.println("Light is ON");
                digitalWrite(outputPin, HIGH);
            } else {
                Serial.println("Light is OFF");
                digitalWrite(outputPin, LOW);
            }
        });
        this->setOnTurnOnCallback([this]() {
            Serial.println("Light is ON");
            lightSince = millis();
            digitalWrite(outputPin, HIGH);
        });
        this->setOnTurnOffCallback([this]() {
            Serial.println("Light is OFF");
        });
    }

    String key() override {
        return keyString;
    }

    void loop() const {
        if (millis() - lightSince > LIGHT_DELAY) {
            digitalWrite(outputPin, LOW);
        }
    }

private:
    unsigned long lightSince = 0;
    int outputPin;
    String keyString;
};


#endif //ESPNOW_POC_LIGHTMANAGER_H
