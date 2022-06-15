#ifndef MESH_LIGHTS_ACTION_H
#define MESH_LIGHTS_ACTION_H

#include <utility>

#include "Arduino.h"
#include "ArduinoJson.h"
#include "Constants.h"

class Action {
public:
    enum STATE {
        ON, OFF
    };

    static const unsigned long WITHOUT_TIMEOUT = 0;

    Action(String device, STATE state, unsigned long timeout = WITHOUT_TIMEOUT) : device(std::move(device)),
                                                                                  state(state),
                                                                                  timeout(timeout) {}

    String device;
    STATE state;
    unsigned long timeout;

    static Action deserialize(JsonDocument &json) {
        return Action({json.getMember("name"), json.getMember("state").as<STATE>(),
                       json.getMember("timeout").as<unsigned long>()});
    }

    DynamicJsonDocument serialize() const {
        DynamicJsonDocument doc(256);
        doc["name"] = device;
        doc["state"] = state;
        doc["timeout"] = timeout;
        return doc;
    }

    String serializeToString() const {
        auto json = this->serialize();
        String msg;
        serializeJson(json, msg);
        return msg;
    }
};

#endif //MESH_LIGHTS_ACTION_H
