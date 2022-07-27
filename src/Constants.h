#ifndef MESH_LIGHTS_CONSTANTS_H
#define MESH_LIGHTS_CONSTANTS_H

// CONFIGURATION
#define LED_PIN                 27          // depends on device
#define LED_RING_PIN            14          // depends on device
#define MOTION_SENSOR_PIN       25          // depends on device
#define LIGHT_KEY               "LIGHT2"    // depends on device
#define NFC_KEY                 "NFC2"    // depends on device
#define MOTION_KEY              "MOTION2"   // depends on device

// I2S PINS
#define BCLK_PIN                22
#define WCLK_PIN                21
#define DOUT_PIN                23

#define DEFAULT_VOLUME          100         // percentage

#define BUTTON_PIN              0

// MESH
#define MESH_PREFIX             "lights"
#define MESH_PASSWORD           "mesh_password"
#define MESH_PORT               5555

#define BROADCAST_LIGHT_KEY     "BLIGHT"
#define BROADCAST_AUDIO_KEY     "BAUDIO"

#endif //MESH_LIGHTS_CONSTANTS_H
