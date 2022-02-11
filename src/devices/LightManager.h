#ifndef ESPNOW_POC_LIGHTMANAGER_H
#define ESPNOW_POC_LIGHTMANAGER_H

#include <memory>
#include "AbsDeviceManager.h"

class LightManager : public AbsDeviceManager {
public:
    explicit LightManager(int outputPin) : outputPin(outputPin) {}

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
            digitalWrite(outputPin  , HIGH);
        });
        this->setOnTurnOffCallback([this]() {
            Serial.println("Light is OFF");
            digitalWrite(outputPin, LOW);
        });
    }

    String key() override {
        return "light";
    }

private:
    int outputPin;
};


#endif //ESPNOW_POC_LIGHTMANAGER_H
