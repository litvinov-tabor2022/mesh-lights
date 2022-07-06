#include "Player.h"

AudioPlayer::Player::Player(AudioGenerator *audioGenerator,
                            AudioOutput *audioOutput,
                            AudioFileSource *fileSource,
                            float volume) :
        audioGenerator(audioGenerator),
        audioOutput(audioOutput),
        fileSource(fileSource),
        volume(volume) {
    init();
}


void AudioPlayer::Player::init() {
    this->audioGenerator->begin(this->fileSource, this->audioOutput);
}

/**
 * Routine for reading and reproducing samples.
 * */
void AudioPlayer::Player::play() {
    Core1.loop("sound", [this] {
        if (!audioGenerator->loop()) {
            audioGenerator->stop();
            playingUninterruptible = false;
            isPlaying = false;
        }
        if (!audioGenerator->isRunning()) {
            audioGenerator->stop();
            audioGenerator->desync();
            playingUninterruptible = false;
            isPlaying = false;
            playNext();
        }
    });
}

bool AudioPlayer::Player::setVolume(int newVolume) {
    if (newVolume < 0 || newVolume > 200) {
        return false;
    }
    volume = (float) newVolume / 100;
    audioOutput->SetGain(volume);
    return true;
}

void AudioPlayer::Player::selectFile(const Sound &sound) {
    playingUninterruptible = sound.uninterruptible;
    fileSource->open(sound.path.c_str());
    if (!audioGenerator->isRunning()) {
        audioGenerator->begin(fileSource, audioOutput);
    }
}

void AudioPlayer::Player::enqueueFile(const std::string &path, bool uninterruptible) {
    std::lock_guard<std::mutex> lock(queueMutex);
    fileQueue.push(Sound(path, uninterruptible));
}

void AudioPlayer::Player::playNext() {
    std::lock_guard<std::mutex> lock(queueMutex);

    if (fileQueue.empty()) {
        isPlaying = false;
        return;
    }
    selectFile(fileQueue.front());
    fileQueue.pop();
    isPlaying = true;
}

void AudioPlayer::Player::playFile(const std::string &path, bool uninterruptible) {
    if (playingUninterruptible) {
        std::lock_guard<std::mutex> lock(queueMutex);
        fileQueue.push(Sound(path, uninterruptible));
    } else {
        selectFile(Sound(path, uninterruptible));
    }
}

void AudioPlayer::Player::stop() {
    this->audioGenerator->desync();
    this->audioGenerator->stop();
    this->audioOutput->stop();
}

bool AudioPlayer::Player::playing() const {
    return this->isPlaying;
}
