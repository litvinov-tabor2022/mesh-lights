#include <Arduino.h>
#include <painlessMesh.h>
#include "Constants.h"
#include "StateManager.h"
#include "devices/LightManager.h"

#ifndef DEVICE_NAME
#define DEVICE_NAME "UNKNOWN"
#endif

StateManager stateManager(DEVICE_NAME);
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
std::shared_ptr<LightManager> lightManager;
bool prevState = false;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendEvent(const Action &action) {
    DynamicJsonDocument message(1024);
    String msg;
    serializeJson(action.serialize(), msg);
    stateManager.processAction(action);
    mesh.sendBroadcast(msg);
}

void sendMessage() {
    Serial.println("Sending event");
    prevState = !prevState;
    sendEvent(Action("event2", prevState ? Action::ON : Action::OFF));
    taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
    DynamicJsonDocument message(1024);
    deserializeJson(message, msg);
//    JsonArray devices = message.getMember("devices").as<JsonArray>();
//    for (auto device: devices) {
    Serial.printf("name: %s, state: %u\n", message.getMember("name").as<String>().c_str(),
                  message.getMember("state").as<Action::STATE>());
    stateManager.processAction(Action::deserialize(message));
//    }
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
    Serial.begin(115200);

    mesh.setDebugMsgTypes(
            ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE); // all types on
//    mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

    lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, "light2"));
    lightManager->init();
    stateManager.registerDeviceManager(lightManager, "event2");
    stateManager.registerDeviceManager(lightManager, "event3");
    stateManager.registerDeviceManager(lightManager, "root");
    Serial.println("Light manager initialized");

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

#ifdef ROOT
    Serial.println("ROOT NODE -----");
    mesh.setRoot(true);
    userScheduler.addTask(taskSendMessage);
    taskSendMessage.enable();
#endif
    mesh.setContainsRoot(true);

    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

}

void loop() {
    // it will run the user scheduler as well
    mesh.update();
}
