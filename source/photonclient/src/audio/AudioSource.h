//
// Created by carl on 20-4-3.
//
#pragma once

#include "AudioFrameFormat.h"
#include <functional>

class AudioSource {
public:
    using OnAudioFrameCallback = std::function<void(const int16_t* input, uint32_t frameCount)>;
    AudioSource(int path, const MicConf& conf, double latency = 0.0);

    ~AudioSource();

    void SetOnAudioFrame(const OnAudioFrameCallback& cb);

    bool StartCapture();

    void StopCapture();

private:
    class Impl;
    Impl* impl_;
};
