#ifndef ESPNOW_POC_ACTION_H
#define ESPNOW_POC_ACTION_H

#include "Arduino.h"
#include "ArduinoJson.h"

struct Action {
    Action(const String &device, bool state) : device(device), state(state) {}

    String device;
    bool state;

    static Action deserialize(ArduinoJson6185_D1::VariantRef *json) {
        return Action({json->getMember("name"), json->getMember("state").as<bool>()});
    }

    [[nodiscard]] DynamicJsonDocument serialize() const {
        DynamicJsonDocument doc(256);
        doc["name"] = device;
        doc["state"] = state;
        return doc;
    }
};

#endif //ESPNOW_POC_ACTION_H
