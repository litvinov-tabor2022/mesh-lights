#ifndef ESPNOW_POC_FIRMWAREDOWNLOADER_H
#define ESPNOW_POC_FIRMWAREDOWNLOADER_H

#include "ESPAsyncWebServer.h"
#include "Constants.h"

class FirmwareDownloader {
public:
    void init() {
        if (SPIFFS.exists(FIRMWARE_FILENAME) && SPIFFS.exists(MD5_FIRMWARE_FILENAME)) {
            this->md5firmware = SPIFFS.open(MD5_FIRMWARE_FILENAME, "r");
            Serial.printf("Nodes firmware fingerprint: %s\n", md5firmware.readString().c_str());
            this->md5firmware.close();
        }
    }

    void handleResponse(AsyncWebServerRequest *request) {
        request->send(statusCode, "text/plain", responseMessage);
    }

    void
    handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            this->firmwareFile = SPIFFS.open(FIRMWARE_FILENAME, "w");
            md5.begin();
            Serial.println("Start downloading new firmware for nodes.");
        }
        if (firmwareFile.write(data, len) != len) {
            Serial.println("Downloading firmware failed");
            this->responseMessage = "Downloading firmware failed";
        }
        md5.add(data, len);
        if (final) {
            firmwareFile.close();
            this->statusCode = 200;
            this->responseMessage = "Downloading firmware was successfully done";
            md5.calculate();
            this->md5firmware = SPIFFS.open(MD5_FIRMWARE_FILENAME, "w");
            String md5String = md5.toString();
            this->md5firmware.write(md5String.c_str());
            this->md5firmware.close();
            Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
        }
    }

private:
    MD5Builder md5{};
    File firmwareFile;
    File md5firmware;
    String responseMessage;
    int statusCode = 500;
};


#endif //ESPNOW_POC_FIRMWAREDOWNLOADER_H
