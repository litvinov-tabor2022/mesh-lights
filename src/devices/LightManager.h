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

    bool init() override {
        Serial.println("Initializing Light manager...");
        ledcAttachPin(outputPin, PWM1_Ch);
        ledcSetup(PWM1_Ch, PWM1_Freq, PWM1_Res);

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

        breathTask.set(TASK_MILLISECOND * 10, TASK_FOREVER, [this]() {
            if (isBreathing) {
                if (PWM1_DutyCycle < 125 && !dimming) {
                    ledcWrite(PWM1_Ch, (PWM1_DutyCycle++) % 125);
                } else {
                    dimming = true;
                }
                if (PWM1_DutyCycle > 0 && dimming) {
                    ledcWrite(PWM1_Ch, PWM1_DutyCycle--);
                } else {
                    dimming = false;
                }
            }
        });
        scheduler->addTask(breathTask);
        breathTask.enable();


        candleTask.set(TASK_MILLISECOND * 10, TASK_FOREVER, [this]() {
            if (isOn) {
                ledcWrite(PWM1_Ch, random(120) + 135);
            } else {
                if (!isBreathing)ledcWrite(PWM1_Ch, 0);
            }
            candleTask.enableDelayed(random(250));
        });
        scheduler->addTask(candleTask);
        candleTask.enable();

        return true;
    }

    void breath(bool state) {
        isBreathing = state;
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
        isOn = true;
    }

    void turnOffLight() {
        Serial.println("Light is OFF");
        isOn = false;
    }

    void timeoutHandler() {
        Serial.println("Timeout");
        turnOffLight();
    }

    unsigned long lightSince = 0;
    int outputPin;
    String keyString;
    Scheduler *scheduler;
    Task breathTask;
    Task candleTask;
    bool isBreathing = false;
    int PWM1_Ch = 0;
    int PWM1_Res = 8;
    int PWM1_Freq = 1000;
    int PWM1_DutyCycle = 0;
    bool dimming = false;
    bool isOn = false;
};


#endif //MESH_LIGHTS_LIGHTMANAGER_H
