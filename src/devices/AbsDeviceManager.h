#ifndef MESH_LIGHTS_ABSDEVICEMANAGER_H
#define MESH_LIGHTS_ABSDEVICEMANAGER_H

#include "Arduino.h"
#include <functional>
#include <optional>
#include <mutex>
#include "Action.h"

class AbsDeviceManager {
public:
    void initTimeout(Scheduler *scheduler, unsigned long timeout = 1000) {
        timeouter.set(TASK_MILLISECOND * timeout, TASK_ONCE, [this]() {
            if (onTimeout.has_value())
                onTimeout.value()();
        });
        scheduler->addTask(timeouter);
        timeouter.enable();
    }

    void setTimeout(unsigned long timeout) {
        Serial.printf("Setting timeout to: %lu\n", timeout);
        timeouter.setInterval(TASK_MILLISECOND * timeout);
        timeouter.restart();
        timeouter.enableDelayed(TASK_MILLISECOND * timeout);

    }

    [[nodiscard]] bool getState() const {
        return state;
    }

    void setState(Action::STATE newState) {
        state = newState;
        if (onStateChange.has_value())
            onStateChange.value()(state);
    }

    Action::STATE toggle() {
        state == Action::ON ? setState(Action::OFF) : setState(Action::ON);
        if (onToggle.has_value())
            onToggle.value()(state);
        return state;
    }

    void turnOn() {
        setState(Action::ON);
        if (onTurnOn.has_value())
            onTurnOn.value()();
    }

    void turnOff() {
        setState(Action::OFF);
        if (onTurnOff.has_value())
            onTurnOff.value()();
    }

    void setOnTurnOnCallback(const std::function<void(void)> &callback) {
        this->onTurnOn.emplace(callback);
    }

    void setOnTurnOffCallback(const std::function<void(void)> &callback) {
        this->onTurnOff.emplace(callback);
    }

    void setOnToggleCallback(const std::function<void(bool)> &callback) {
        this->onToggle.emplace(callback);
    }

    void setOnStateChange(const std::function<void(Action::STATE)> &callback) {
        this->onStateChange.emplace(callback);
    }

    void setOnTimeout(const std::function<void()> &callback) {
        this->onTimeout.emplace(callback);
    }

    Action getAction() {
        return Action({key(), state});
    }

    virtual String key() = 0;

private:
    std::optional<std::function<void(void)>> onTurnOn;
    std::optional<std::function<void(void)>> onTurnOff;
    std::optional<std::function<void(Action::STATE)>> onToggle;
    std::optional<std::function<void(Action::STATE)>> onStateChange;
    std::optional<std::function<void(void)>> onTimeout;
    Action::STATE state = Action::STATE::OFF;
    Task timeouter;
};


#endif //MESH_LIGHTS_ABSDEVICEMANAGER_H
