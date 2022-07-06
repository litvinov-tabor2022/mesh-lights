#ifndef MESH_LIGHTS_AUDIOMANAGER_H
#define MESH_LIGHTS_AUDIOMANAGER_H

#include <utility>

#include "audio/Player.h"
#include "AbsDeviceManager.h"
#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"

class AudioManager : public AbsDeviceManager {
public:
    bool init() override {
        SPIFFS.begin();
        audioOutput.SetPinout(BCLK_PIN, WCLK_PIN, DOUT_PIN);
        audioPlayer = new AudioPlayer::Player(&mp3, &audioOutput, &source, (DEFAULT_VOLUME / 100.0));
        audioPlayer->setVolume(DEFAULT_VOLUME);
        audioPlayer->play();

        this->setOnTurnOnCallback([this]() {
            audioPlayer->enqueueFile("/ring.mp3");
        });

        return true;
    }

    String key() override {
        return keyString;
    }

private:
    String keyString;
    AudioOutputI2S audioOutput;
    AudioGeneratorMP3 mp3;
    AudioFileSourceSPIFFS source;
    AudioPlayer::Player *audioPlayer;

};

#endif //MESH_LIGHTS_AUDIOMANAGER_H
