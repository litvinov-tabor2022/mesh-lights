#ifndef MESH_LIGHTS_CALLBACKS_H
#define MESH_LIGHTS_CALLBACKS_H

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

class WebServerCallback {
public:
    static void init(AsyncWebServer &server) {
//        server.on("/message", HTTP_GET, [](AsyncWebServerRequest *request) {
//            request->send(200, "text/html",
//                          "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
//            if (request->hasArg("BROADCAST")) {
//                String msg = request->arg("BROADCAST");
//                if (msg == "on") {
//                    lightManager->turnOn();
//                }
//                if (msg == "off") {
//                    lightManager->turnOff();
//                }
//                sendmsg();
//            }
//        });
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/index.html", "text/html", false, NULL);
        });
        server.on("/controller", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/controller.html", "text/html", false, NULL);
        });
    }
};

#endif //MESH_LIGHTS_CALLBACKS_H
