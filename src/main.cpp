#include <Arduino.h>
#include <painlessMesh.h>
#include "Constants.h"
#include "StateManager.h"

#ifdef ROOT

#include "nodes/RootNode.h"

#else

#include "nodes/SlaveNode.h"

#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "UNKNOWN"
#endif

Node *node;
StateManager stateManager(DEVICE_NAME);
Scheduler userScheduler; // to control your personal task
Scheduler meshScheduler;
LedRingManager ledRingManager(LED_RING_PIN, &meshScheduler);
NfcManager nfcManager(NFC_KEY, &ledRingManager, &meshScheduler);
painlessMesh mesh;

// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
    DynamicJsonDocument message(1024);
    deserializeJson(message, msg);
    Serial.printf("Received from %u, action name: %s, state: %u, timeout %lu\n", from,
                  message.getMember("name").as<String>().c_str(),
                  message.getMember("state").as<Action::STATE>(),
                  message.getMember("timeout").as<unsigned long>());
    stateManager.processAction(Action::deserialize(message));
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
    Serial.begin(115200);
    userScheduler.init();

#ifdef NFC
    if (!nfcManager.init()) {
        Serial.println("Could not initialize NFC manager!");
        return;
    }

    if (!ledRingManager.init()) {
        Serial.println("Could not initialize LED ring!");
        return;
    }

    ledRingManager.displayOff();
#endif

#ifdef ROOT
    Serial.println("ROOT NODE -----");
    node = new RootNode();
#else
    Serial.println("SLAVE NODE -----");
    node = new SlaveNode();
#endif

#ifdef ROOT
    mesh.setDebugMsgTypes(
            ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE); // all types on
#else
    mesh.setDebugMsgTypes(
            ERROR | STARTUP | MESH_STATUS | CONNECTION);  // set before init() so that you can see startup messages
#endif
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &meshScheduler, MESH_PORT);
    node->init(&mesh, &userScheduler, &nfcManager, &stateManager, LED_PIN);

#ifdef ROOT
    mesh.setRoot(true);
#endif
    mesh.setContainsRoot(true);

    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    userScheduler.enable();
}

void loop() {
    // it will run the user scheduler as well
    mesh.update();
    userScheduler.execute();
}
