#ifndef MESH_LIGHTS_SLAVENODE_H
#define MESH_LIGHTS_SLAVENODE_H

#include <memory>
#include <utility>
#include "Node.h"
#include "devices/LightManager.h"
#include "devices/MotionSensorManager.h"
#include "devices/AudioManager.h"
#include "devices/NfcManager.h"

class SlaveNode : public Node {
public:
    bool init(painlessMesh *mesh, Scheduler *scheduler, StateManager *stateManager, int statusLedPin) override {
        if (!Node::init(mesh, scheduler, stateManager, statusLedPin))
            return false;

        this->lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, LIGHT_KEY, scheduler));
        this->motionSensorManager =
                std::shared_ptr<MotionSensorManager>(new MotionSensorManager(MOTION_SENSOR_PIN, MOTION_KEY, scheduler));
        this->audioManager = std::shared_ptr<AudioManager>(new AudioManager());
        this->nfcManager = std::shared_ptr<NfcManager>(new NfcManager(NFC_KEY, scheduler));

        lightManager->init();
        motionSensorManager->init();
        audioManager->init();
        nfcManager->init();

        stateManager_->registerDeviceManager(lightManager, BROADCAST_LIGHT_KEY)
                ->registerDeviceManager(audioManager, MOTION_KEY)
                ->registerDeviceManager(lightManager, LIGHT_KEY)
                ->registerDeviceManager(nfcManager, NFC_KEY)
                ->registerDeviceManager(motionSensorManager, MOTION_KEY);

        unlocker.set(TASK_SECOND * 10, TASK_ONCE, [this]() {
            isMotionSensorLocked = false;
        });

        buttonClickTask.set(TASK_MILLISECOND * 100, TASK_FOREVER, [this]() {
            if (digitalRead(BUTTON_PIN) == LOW) {
                String msg = Action(BROADCAST_LIGHT_KEY, Action::STATE::ON, 3000).serializeToString();
                Serial.printf("Turning all lights on, %s\n", msg.c_str());
                mesh_->sendBroadcast(msg);
            }
            if (mesh_->getNodeList(false).empty()) {
                lightManager->breath(true);
            } else {
                lightManager->breath(false);
            }
        });
        statusTask.set(TASK_MILLISECOND * 250, TASK_FOREVER, [this]() {

        });

        this->nfcManager->setOnTurnOnCallback([this]() {
            String msg = Action(BROADCAST_LIGHT_KEY, Action::STATE::ON, 3000).serializeToString();
            Serial.printf("Turning all lights on, %s\n", msg.c_str());
            mesh_->sendBroadcast(msg);
        });

        // motion sensor event throttling
        this->motionSensorManager->setOnStateChange([this](Action::STATE state) {
            if (!isMotionSensorLocked && state == Action::STATE::ON) {
                isMotionSensorLocked = true;
                unlocker.setInterval(TASK_SECOND * 5);
                unlocker.restart();
                unlocker.enableDelayed(TASK_SECOND * 5);
                sendEvent();
            }
        });

        scheduler_->addTask(unlocker);
        scheduler_->addTask(buttonClickTask);
        unlocker.enable();
        buttonClickTask.enable();
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

    Task unlocker;
    Task buttonClickTask;
    Task statusTask;
    bool isMotionSensorLocked = false;
    std::shared_ptr<LightManager> lightManager;
    std::shared_ptr<MotionSensorManager> motionSensorManager;
    std::shared_ptr<AudioManager> audioManager;
    std::shared_ptr<NfcManager> nfcManager;
};


#endif //MESH_LIGHTS_SLAVENODE_H
