#ifndef ESPNOW_POC_ACTION_H
#define ESPNOW_POC_ACTION_H

#include <utility>

#include "Arduino.h"
#include "ArduinoJson.h"

struct Action {
    enum STATE {
        ON, OFF
    };

    Action(String device, STATE state) : device(std::move(device)), state(state) {}

    String device;
    STATE state;

    static Action deserialize(JsonDocument &json) {
        return Action({json.getMember("name"), json.getMember("state").as<STATE>()});
    }

    [[nodiscard]] DynamicJsonDocument serialize() const {
        DynamicJsonDocument doc(256);
        doc["name"] = device;
        doc["state"] = state;
        return doc;
    }
};

#endif //ESPNOW_POC_ACTION_H
