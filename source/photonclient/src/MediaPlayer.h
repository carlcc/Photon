//
// Created by carl on 20-4-1.
//
#pragma once

#include "audio/AudioFrameFormat.h"
#include "video/IVideoFrame.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <memory>

class MediaPlayer {
public:
    struct Config {
        const char* title { "" };
        int x { 0 };
        int y { 0 };
        int width { 640 };
        int height { 480 };
    };

    explicit MediaPlayer(const Config* config);

    virtual ~MediaPlayer();

    bool Run();

    void Stop();

    void SetVideoFrame(const std::shared_ptr<IVideoFrame>& frame);

    void SetAudioFormat(const MicConf& format);

    uint32_t PushAudioFrames(const int16_t* data, uint32_t count);

    virtual void UpdateUI() {}

private:
    class Impl;
    Impl* impl_;
};
