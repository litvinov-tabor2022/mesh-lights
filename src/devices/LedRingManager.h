#ifndef MESH_LIGHTS_LEDRINGMANAGER_H
#define MESH_LIGHTS_LEDRINGMANAGER_H

#include <memory>
#include <utility>
#include "AbsDeviceManager.h"
#include "Adafruit_NeoPixel.h"
#include <painlessMesh.h>

#define LEDRING_BRIGHTNESS 255
#define LEDRING_BITS 16

#define LEDRING_COLOR_RED Adafruit_NeoPixel::Color(150, 0, 0)
#define LEDRING_COLOR_GREEN Adafruit_NeoPixel::Color(0, 150, 0)
#define LEDRING_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 150)
#define LEDRING_COLOR_ORANGE Adafruit_NeoPixel::Color(255,165,0)

class LedRingManager : public AbsDeviceManager {
public:
    explicit LedRingManager(int outputPin) : outputPin(outputPin) {}

    LedRingManager(int outputPin, Scheduler *scheduler) : outputPin(outputPin), scheduler(scheduler) {
        initTimeout(scheduler);
    }

    bool init() override {
        Serial.println("Initializing LED ring manager...");

        std::lock_guard<std::mutex> lg(mutex);
        pixels = new Adafruit_NeoPixel(LEDRING_BITS, outputPin, NEO_GRB + NEO_KHZ800);

        pixels->begin();
        pixels->setBrightness(LEDRING_BRIGHTNESS);
        pixels->clear();
        pixels->show();

        return true;
    }

    void displayOff() {
        Serial.println("LED ring switch to off state");
        std::lock_guard<std::mutex> lg(mutex);

        for (int i = 0; i < LEDRING_BITS; i++) {
            pixels->setPixelColor(i, LEDRING_COLOR_RED);
        }
        pixels->show();
    }

    void countdown(int secs) {
        Serial.printf("LED ring countdown for %d secs\n", secs);

        // TODO
        for (int i = 0; i < LEDRING_BITS; i++) {
            pixels->setPixelColor(i, LEDRING_COLOR_GREEN);
        }
        pixels->show();

        countdownRest = LEDRING_BITS - 1;

        const int period = floor((float) secs * 1000 / (float) LEDRING_BITS);

        countdownTask.set(TASK_MILLISECOND * period, LEDRING_BITS, [this]() {
            std::lock_guard<std::mutex> lg(mutex);
            pixels->clear();
            for (int i = 0; i < countdownRest; i++) {
                pixels->setPixelColor(i, LEDRING_COLOR_GREEN);
            }
            for (int i = countdownRest; i < LEDRING_BITS; i++) {
                pixels->setPixelColor(i, LEDRING_COLOR_RED);
            }
            pixels->show();

            countdownRest--;

        });
        scheduler->addTask(countdownTask);
        countdownTask.enable();
    }


    String key() override {
        return "";
    }


private:
    void turnOnLight() {
    }

    void turnOffLight() {
    }

    void timeoutHandler() {
        Serial.println("Timeout");
        turnOffLight();
    }

    std::mutex mutex;
    Adafruit_NeoPixel *pixels;
    int outputPin;

    Scheduler *scheduler;
    Task countdownTask;

    uint8_t countdownRest = LEDRING_BITS;
};


#endif //MESH_LIGHTS_LEDRINGMANAGER_H
