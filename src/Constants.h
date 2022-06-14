#ifndef ESPNOW_POC_CONSTANTS_H
#define ESPNOW_POC_CONSTANTS_H


static const uint8_t BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define   MESH_PREFIX           "lights"
#define   MESH_PASSWORD         "mesh_password"
#define   MESH_PORT             5555
#define   OTA_PART_SIZE         1024

#define   LED_PIN               LED_BUILTIN

//#define   STATION_SSID     "LightsEntrypoint"
//#define   STATION_PASSWORD "1234567890"

// ROOT NODE WILL BE CONNECTED TO THIS NETWORK
#define   STATION_SSID          "***REMOVED***"
#define   STATION_PASSWORD      "***REMOVED***"
#define   HOSTNAME              "root_node"

#define FIRMWARE_FILENAME       "node.bin"
#define MD5_FIRMWARE_FILENAME   "node.md5"

#endif //ESPNOW_POC_CONSTANTS_H
