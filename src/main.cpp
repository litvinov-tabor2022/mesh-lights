#include "IPAddress.h"
#include "painlessMesh.h"
#include "ESPAsyncWebServer.h"
#include <ESP8266WiFi.h>
#include "Constants.h"
#include "FirmwareDownloader.h"
#include "AsyncJson.h"
#include "StateManager.h"
#include "devices/LightManager.h"
#include <ESP8266mDNS.h>

StateManager stateManager;
std::shared_ptr<LightManager> lightManager;
Scheduler userScheduler;
AsyncWebServer server(80);
painlessMesh mesh;
FirmwareDownloader firmwareDownloader;
//bool lightState = false;

IPAddress myAPIP(0, 0, 0, 0);

void sendmsg();

Task taskSendmsg(TASK_SECOND * 1, TASK_FOREVER, &sendmsg);

void sendmsg() {
    if (mesh.getNodeList(false).empty()) {
        taskSendmsg.setInterval(250);
        if (taskSendmsg.getRunCounter() & 1) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    } else if (mesh.isRoot()) {
        DynamicJsonDocument message(1024);
        taskSendmsg.setInterval(TASK_SECOND * 1);
        String msg;
        serializeJson(stateManager.serialize(), msg);
        mesh.sendBroadcast(msg);
        taskSendmsg.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
    }
}


void receivedCallback(uint32_t from, String &msg) {
    DynamicJsonDocument message(1024);
    deserializeJson(message, msg);
    JsonArray devices = message.getMember("devices").as<JsonArray>();
    for (auto device: devices) {
        stateManager.processAction(Action::deserialize(&device));
    }
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

void
handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
//    Serial.printf("UploadStart: %s\n", filename.c_str());
//    if (!index) {
//        File f = SPIFFS.open("example.txt", "w");
//        Serial.println("Start downloading new firmware for nodes.");
//    }
//    mesh.initOTASend([&](painlessmesh::plugin::ota::DataRequest pkg, char *buffer) {
//        memcpy(buffer, data, OTA_PART_SIZE * pkg.partNo);
//        return min((unsigned) OTA_PART_SIZE, (unsigned) len - (OTA_PART_SIZE * pkg.partNo));
//    }, OTA_PART_SIZE);
//    MD5Builder md5;
//    md5.begin();
//    md5.add(data, len);
//    md5.calculate();
//
//    mesh.offerOTA("node", "ESP8266", md5.toString(),
//                  ceil(((float) len) / OTA_PART_SIZE), false);
//
//
//    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS INIT ERROR");
    }

    lightManager = std::shared_ptr<LightManager>(new LightManager(LED_PIN));
    lightManager->init();
    stateManager.registerDeviceManager(lightManager);
    Serial.println("Light manager initialized");

    firmwareDownloader.init();
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

    mesh.setContainsRoot(true);
    userScheduler.addTask(taskSendmsg);

#ifdef ROOT
    server.on("/message", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",
                      "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
        if (request->hasArg("BROADCAST")) {
            String msg = request->arg("BROADCAST");
            if (msg == "on") {
                lightManager->turnOn();
            }
            if (msg == "off") {
                lightManager->turnOff();
            }
            sendmsg();
        }
    });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html", false, NULL);
    });
    server.on("/controller", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/controller.html", "text/html", false, NULL);
    });
    server.on("/", HTTP_POST, [&](AsyncWebServerRequest *request) {
                  firmwareDownloader.handleResponse(request);
              },
              [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len,
                  bool final) {
                  firmwareDownloader.handleUpload(request, filename, index, data, len, final);
              });
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/controller",
                                                                           [&](AsyncWebServerRequest *request,
                                                                               JsonVariant &json) {
                                                                               JsonObject jsonObj = json.as<JsonObject>();
                                                                               int statusCode = 400;
                                                                               if (jsonObj["lightState"] == "on") {
                                                                                   lightManager->turnOn();
                                                                                   statusCode = 200;
                                                                               } else if (jsonObj["lightState"] ==
                                                                                          "off") {
                                                                                   lightManager->turnOff();
                                                                                   statusCode = 200;
                                                                               }
                                                                               sendmsg();
                                                                               request->send(statusCode);
                                                                           });
    server.addHandler(handler);
    server.begin();
//    mesh.initOTAReceive("root");
#else
    mesh.initOTAReceive("node");
#endif
    taskSendmsg.enable();
}

void loop() {
    mesh.update();
//    MDNS.update();
//    if (!mesh.getNodeList(false).empty()) digitalWrite(LED_PIN, !lightState);
}