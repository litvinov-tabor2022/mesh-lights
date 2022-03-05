#ifndef ESPNOW_POC_ABSDEVICEMANAGER_H
#define ESPNOW_POC_ABSDEVICEMANAGER_H


#include <functional>
#include <mutex>
#include "Arduino.h"
#include "Action.h"

// TODO call callbacks asynchronously
class AbsDeviceManager {
public:
    //TODO add handling requests from web

    [[nodiscard]] bool getState() const {
        return state;
    }

    void setState(Action::STATE s) {
        AbsDeviceManager::state = s;
    }

    bool toggle() {
        state == Action::ON ? state = Action::OFF : state = Action::ON;
        onToggle(state);
        return state;
    }

    void turnOn() {
        state = Action::ON;
        onTurnOn();
    }

    void turnOff() {
        state = Action::OFF;
        onTurnOff();
    }

    void setOnTurnOnCallback(const std::function<void(void)> &callback) {
        this->onTurnOn = callback;
    }

    void setOnTurnOffCallback(const std::function<void(void)> &callback) {
        this->onTurnOff = callback;
    }

    void setOnToggleCallback(const std::function<void(bool)> &callback) {
        this->onToggle = callback;
    }

    Action getAction() {
        return Action({key(), state});
    }

    virtual String key() = 0;

private:
    std::function<void(void)> onTurnOn;
    std::function<void(void)> onTurnOff;
    std::function<void(bool)> onToggle;
    Action::STATE state = Action::STATE::OFF;
};


#endif //ESPNOW_POC_ABSDEVICEMANAGER_H
