#ifndef MESH_LIGHTS_ROOTNODE_H
#define MESH_LIGHTS_ROOTNODE_H

#include <memory>
#include <utility>
#include "Node.h"
#include "devices/LightManager.h"
#include "devices/MotionSensorManager.h"

class RootNode : public Node {
public:

    bool init(painlessMesh *mesh, Scheduler *scheduler, StateManager *stateManager, int statusLedPin) override {
        if (!Node::init(mesh, scheduler, stateManager, statusLedPin))
            return false;

        pinMode(BUTTON_PIN, INPUT_PULLUP);
        pinMode(LED_PIN, OUTPUT);
        pinMode(MOTION_SENSOR_PIN, INPUT);

        this->lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, LIGHT_KEY, scheduler));
        this->motionSensorManager =
                std::shared_ptr<MotionSensorManager>(new MotionSensorManager(MOTION_SENSOR_PIN, MOTION_KEY, scheduler));
        lightManager->init();
        motionSensorManager->init();



        stateManager_
                ->registerDeviceManager(lightManager, BROADCAST_LIGHT_KEY)
                ->registerDeviceManager(lightManager, LIGHT_KEY)
                ->registerDeviceManager(motionSensorManager, MOTION_KEY);

        this->motionSensorManager->setOnStateChange([this](Action::STATE state) {
            sendEvent();
        });

        buttonClickTask.set(TASK_MILLISECOND * 100, TASK_FOREVER, [this]() {
            if (digitalRead(BUTTON_PIN) == LOW) {
                String msg = Action(BROADCAST_LIGHT_KEY, Action::STATE::ON, 10000).serializeToString();
                Serial.printf("Turning all lights on, %s\n", msg.c_str());
                mesh_->sendBroadcast(msg);
            }
        });

        statusPrinter.set(TASK_SECOND * 5, TASK_FOREVER, [this]() {
            Serial.println(mesh_->subConnectionJson());
        });

        scheduler_->addTask(buttonClickTask);
        scheduler_->addTask(statusPrinter);
        statusPrinter.enable();
        buttonClickTask.enable();
        return true;
    };

private:
    void sendEvent() {
        auto action = motionSensorManager->getAction();
        DynamicJsonDocument message(1024);
        String msg;
        serializeJson(action.serialize(), msg);
        mesh_->sendBroadcast(msg);
    }

    static const int HEARTBEAT_FREQ = 1000;
    Task buttonClickTask;
    Task statusPrinter;
    std::shared_ptr<LightManager> lightManager;
    std::shared_ptr<MotionSensorManager> motionSensorManager;
};


#endif //MESH_LIGHTS_ROOTNODE_H
