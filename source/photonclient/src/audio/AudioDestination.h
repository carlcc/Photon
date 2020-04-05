//
// Created by carl on 2020/4/5 0005.
//
#pragma once

#include "AudioFrameFormat.h"
#include <functional>

class AudioDestination {
public:
    using OnAudioFrameCallback = std::function<void(const int16_t* input, uint32_t frameCount)>;
    AudioDestination(int path, const MicConf& conf, double latency = 0.0);

    ~AudioDestination();

    bool WriteSounds(const void* data, uint32_t frameCount);

    bool StartPlay();

    void StopPlay();

    explicit operator bool() const;

    bool operator!() const
    {
        return !bool(*this);
    }

private:
    class Impl;
    Impl* impl_;
};