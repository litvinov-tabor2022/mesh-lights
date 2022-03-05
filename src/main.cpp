#include "IPAddress.h"
#include "painlessMesh.h"
#include "ESPAsyncWebServer.h"
#include <ESP8266WiFi.h>
#include "Constants.h"
#include "StateManager.h"
#include "devices/LightManager.h"
#include <ESP8266mDNS.h>
#include "Callbacks.h"

StateManager stateManager;
Scheduler userScheduler;
AsyncWebServer server(80);
painlessMesh mesh;

IPAddress myAPIP(0, 0, 0, 0);

void checkConnection();

Task taskSendmsg(TASK_SECOND * 1, TASK_FOREVER, &checkConnection);

void checkConnection() {
    // waiting for connection
    if (mesh.getNodeList(false).empty()) {
        taskSendmsg.setInterval(TASK_MILLISECOND * 250);
        if (taskSendmsg.getRunCounter() & 1) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else {
            digitalWrite(LED_BUILTIN, LOW);
        }
    } else {
        // TODO: uncomment in prod
//        digitalWrite(LED_BUILTIN, HIGH);
    }
    /*else if (mesh.isRoot()) {
        DynamicJsonDocument message(1024);
        taskSendmsg.setInterval(TASK_SECOND * 1);
        String msg;
        serializeJson(stateManager.serialize(), msg);
        mesh.sendBroadcast(msg);
        taskSendmsg.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
        Serial.println(mesh.subConnectionJson(true));
    }*/
}

#if defined SENSOR || ROOT

void sendEvent(const Action &action) {
    DynamicJsonDocument message(1024);
    String msg;
    serializeJson(action.serialize(), msg);
    mesh.sendBroadcast(msg);
}

#endif

void receivedCallback(uint32_t from, String &msg) {
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
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS INIT ERROR");
    }

#ifdef NODE1
    auto lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, "light1"));
    lightManager->init();
    stateManager.registerDeviceManager(lightManager, "event1");
    stateManager.registerDeviceManager(lightManager, "event2");
    stateManager.registerDeviceManager(lightManager, "root");
    Serial.println("Light manager initialized");
#endif
#ifdef NODE2
    auto lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN, "light2"));
    lightManager->init();
    stateManager.registerDeviceManager(lightManager, "event2");
    stateManager.registerDeviceManager(lightManager, "event3");
    stateManager.registerDeviceManager(lightManager, "root");
    Serial.println("Light manager initialized");
#endif
//    mesh.setDebugMsgTypes(
//            ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
#ifdef ROOT
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
    mesh.stationManual(STATION_SSID, STATION_PASSWORD);
    mesh.setHostname(HOSTNAME);
    if (!MDNS.begin("root_node")) {
        Serial.println("Error setting up MDNS responder!");
    } else {
        Serial.println("mDNS responder started");
    }
#else
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
#endif
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

#ifdef ROOT
    Serial.println("I am a root node.");
    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());
    mesh.setRoot(true);
#endif

#ifndef ROOT
    mesh.setContainsRoot(true);
#endif
    userScheduler.addTask(taskSendmsg);

#ifdef ROOT
    WebServerCallback::init(server);
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/controller",
                                                                           [&](AsyncWebServerRequest *request,
                                                                               JsonVariant &json) {
                                                                               JsonObject jsonObj = json.as<JsonObject>();
                                                                               int statusCode = 400;
                                                                               if (jsonObj["state"] == "on") {
                                                                                   sendEvent(
                                                                                           Action(jsonObj["device"],
                                                                                                  Action::ON));
                                                                                   statusCode = 200;
                                                                               } else if (jsonObj["state"] == "off") {
                                                                                   sendEvent(
                                                                                           Action(jsonObj["device"],
                                                                                                  Action::OFF));
                                                                                   statusCode = 200;
                                                                               }
                                                                               checkConnection();
                                                                               request->send(statusCode);
                                                                           });
    server.addHandler(handler);
    server.begin();
//    mesh.initOTAReceive("root");
#else
    //    mesh.initOTAReceive("node");
#endif
    taskSendmsg.enable();
}

void loop() {
    mesh.update();
}