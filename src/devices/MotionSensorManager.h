#ifndef MESH_LIGHTS_MOTIONSENSORMANAGER_H
#define MESH_LIGHTS_MOTIONSENSORMANAGER_H

#include <memory>
#include <utility>
#include "AbsDeviceManager.h"
#include <painlessMesh.h>

class MotionSensorManager : public AbsDeviceManager {
public:
    MotionSensorManager(int inputPin, String key, Scheduler *scheduler) : inputPin(inputPin),
                                                                          keyString(std::move(key)),
                                                                          scheduler(scheduler) {}

    bool init() override {
        Serial.println("Initializing motion sensor manager...");
        pinMode(inputPin, INPUT_PULLUP);
        motionChecker.set(TASK_MILLISECOND * 200, TASK_FOREVER, [this]() {
            motionHandler();
        });
        scheduler->addTask(motionChecker);
        motionChecker.enable();

        return true;
    }

    String key() override {
        return keyString;
    }

private:
    void motionHandler() {
        bool sensorState = digitalRead(inputPin);
        if (sensorState == HIGH) {
            this->turnOn();
        }
    }

    int inputPin;
    String keyString;
    Task motionChecker;
    Scheduler *scheduler;
};

#endif //MESH_LIGHTS_MOTIONSENSORMANAGER_H
