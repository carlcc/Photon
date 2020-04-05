//
// Created by carl on 2020/4/5 0005.
//

#include "AudioDestination.h"
#include <iostream>
#include <portaudio.h>

class AudioDestination::Impl {
public:
    Impl(int path, const MicConf& conf, double latency)
    {
        PaStreamParameters outputParameters {};
        outputParameters.device = path;
        outputParameters.channelCount = conf.channelCount;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = latency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;
        PaError err = Pa_OpenStream(&audioStream_, nullptr, &outputParameters, conf.sampleRate, 0.1 * conf.sampleRate, 0, nullptr, nullptr);
        if (err != paNoError) {
            std::cerr << "can't open audio: " << Pa_GetErrorText(err) << std::endl;
            micConf_ = MicConf {};
            return;
        }
        micConf_ = conf;
    }
    ~Impl()
    {
        if (audioStream_ != nullptr) {
            Pa_CloseStream(audioStream_);
            audioStream_ = nullptr;
        }
    }

    bool WriteSounds(const void* data, uint32_t frameCount)
    {
        if (audioStream_ == nullptr) {
            std::cerr << "Invalid audio destination" << std::endl;
            return false;
        }
        auto err = Pa_WriteStream(audioStream_, data, frameCount);
        if (err != paNoError) {
            std::cerr << "Write sounds error: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }
        return true;
    }

    bool StartPlay()
    {
        if (audioStream_ == nullptr) {
            return false;
        }
        auto err = Pa_StartStream(audioStream_);
        if (err != paNoError) {
            std::cerr << "Start audio destination failed: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }
        return true;
    }

    void StopPlay()
    {
        if (audioStream_ != nullptr) {
            Pa_StopStream(audioStream_);
        }
    }

    explicit operator bool() const
    {
        return audioStream_ != nullptr;
    }

    PaStream* audioStream_ { nullptr };
    MicConf micConf_ {};
};

AudioDestination::AudioDestination(int path, const MicConf& conf, double latency)
    : impl_(new Impl(path, conf, latency))
{
}

AudioDestination::~AudioDestination()
{
    delete impl_;
    impl_ = nullptr;
}

bool AudioDestination::WriteSounds(const void* data, uint32_t frameCount)
{
    return impl_->WriteSounds(data, frameCount);
}

bool AudioDestination::StartPlay()
{
    return impl_->StartPlay();
}

void AudioDestination::StopPlay()
{
    impl_->StopPlay();
}

AudioDestination::operator bool() const
{
    return bool(*impl_);
}
