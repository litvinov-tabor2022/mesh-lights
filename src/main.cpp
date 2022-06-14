#include <Arduino.h>
#include "Constants.h"
#include "StateManager.h"
#include "devices/LightManager.h"
#include <esp_now.h>
#include <WiFi.h>

#ifndef DEVICE_NAME
#define DEVICE_NAME "UNKNOWN"
#endif

esp_now_peer_info_t peerInfo;
StateManager stateManager(DEVICE_NAME);
Scheduler userScheduler; // to control your personal task
std::shared_ptr<LightManager> lightManager;
bool prevState = false;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendEvent(const Action &action) {
    StaticJsonDocument<250> message;
    String msg;
    serializeJson(action.serialize(), msg);
    stateManager.processAction(action);
    esp_err_t result = esp_now_send(BROADCAST_ADDRESS, reinterpret_cast<const uint8_t *>(msg.c_str()), msg.length());
    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }
}

void sendMessage() {
    Serial.println("Sending event");
    prevState = !prevState;
    sendEvent(Action("event2", prevState ? Action::ON : Action::OFF));
    taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void receivedCallback(const uint8_t *mac, const uint8_t *incomingData, int len) {
    Serial.printf("Received from %s msg=%s\n", mac, incomingData);
    DynamicJsonDocument message(250);
    deserializeJson(message, incomingData, len);
//    JsonArray devices = message.getMember("devices").as<JsonArray>();
//    for (auto device: devices) {
    Serial.printf("name: %s, state: %u\n", message.getMember("name").as<String>().c_str(),
                  message.getMember("state").as<Action::STATE>());
    stateManager.processAction(Action::deserialize(message));
//    }
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    lightManager = std::shared_ptr<LightManager>(new LightManager(2, "light2"));
    lightManager->init();
    stateManager.registerDeviceManager(lightManager, "event2");
    stateManager.registerDeviceManager(lightManager, "event3");
    stateManager.registerDeviceManager(lightManager, "root");
    Serial.println("Light manager initialized");

#ifdef ROOT
    Serial.println("ROOT NODE -----");
    memcpy(peerInfo.peer_addr, BROADCAST_ADDRESS, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    } else {
        Serial.println("Peer added");
    }

    userScheduler.addTask(taskSendMessage);
    taskSendMessage.enable();
    userScheduler.startNow();

#else
    esp_now_register_recv_cb(receivedCallback);
#endif
}

void loop() {
    // it will run the user scheduler as well
//    mesh.update();
    userScheduler.execute();
}
