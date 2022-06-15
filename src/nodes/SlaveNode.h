#ifndef MESH_LIGHTS_SLAVENODE_H
#define MESH_LIGHTS_SLAVENODE_H

#include <memory>
#include <utility>
#include "Node.h"
#include "devices/LightManager.h"
#include "devices/MotionSensorManager.h"

class SlaveNode : public Node {
public:
    bool init(painlessMesh *mesh, Scheduler *scheduler, StateManager *stateManager) override {
        if (!Node::init(mesh, scheduler, stateManager))
            return false;

        pinMode(LED_PIN, OUTPUT);

        this->lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, LIGHT_KEY, scheduler));
        this->motionSensorManager =
                std::shared_ptr<MotionSensorManager>(new MotionSensorManager(MOTION_SENSOR_PIN, MOTION_KEY, scheduler));
        lightManager->init();
        motionSensorManager->init();

        stateManager_->registerDeviceManager(lightManager, BROADCAST_LIGHT_KEY);
        stateManager_->registerDeviceManager(lightManager, LIGHT_KEY);
        stateManager_->registerDeviceManager(motionSensorManager, MOTION_KEY);

        this->motionSensorManager->setOnStateChange([this](Action::STATE state) {
            sendEvent();
        });
        return true;
    };

    void begin() override {
    }


private:
    void sendEvent() {
        auto action = motionSensorManager->getAction();
        DynamicJsonDocument message(1024);
        String msg;
        serializeJson(action.serialize(), msg);
        mesh_->sendBroadcast(msg);
    }

    std::shared_ptr<LightManager> lightManager;
    std::shared_ptr<MotionSensorManager> motionSensorManager;
};


#endif //MESH_LIGHTS_SLAVENODE_H
