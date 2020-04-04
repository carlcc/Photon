//
// Created by carl on 20-4-3.
//

#include "AudioSource.h"
#include <iostream>
#include <portaudio.h>

class AudioSource::Impl {
public:
    Impl(int path, const MicConf& conf, double latency)
    {
        if (path == paNoDevice) {
            std::cerr << "Error: No default input device." << std::endl;
            return;
        }

        PaStreamParameters inputParameters { 0 };
        inputParameters.device = path;
        inputParameters.channelCount = conf.channelCount; /* stereo input */
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = latency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        auto err = Pa_OpenStream(
            &stream_,
            &inputParameters,
            nullptr, /* &outputParameters, */
            conf.sampleRate,
            (unsigned long)(conf.sampleRate * 0.01), // 10 ms
            paClipOff, /* we won't output out of range samples so don't bother clipping them */
            Impl::OnStreamCallback,
            this);
        if (err != paNoError) {
            std::cerr << "Open stream failed" << std::endl;
            return;
        }
        micConf_ = conf;
    }

    ~Impl()
    {
        StopCapture(true);
        if (stream_ != nullptr) {
            Pa_CloseStream(stream_);
            stream_ = nullptr;
        }
    }

    static int OnStreamCallback(const void* input, void* output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    {
        auto* self = (Impl*)userData;

        if (self->onAudioFrame_) {
            self->onAudioFrame_((int16_t*)input, uint32_t(frameCount));
        }
        return paContinue;
    }

    void SetOnAudioFrame(const AudioSource::OnAudioFrameCallback& cb)
    {
        onAudioFrame_ = cb;
    }

    bool StartCapture()
    {
        if (stream_ == nullptr) {
            return false;
        }
        if (Pa_IsStreamActive(stream_)) {
            return true;
        }

        auto err = Pa_StartStream(stream_);
        if (err != paNoError) {
            return false;
        }
        return true;
    }

    void StopCapture(bool waitUntilThreadStop)
    {
        if (stream_ == nullptr) {
            return;
        }
        if (Pa_IsStreamStopped(stream_)) {
            return;
        }
        auto err = Pa_StopStream(stream_);
        if (err != paNoError) {
            std::cerr << "Stop stream failed" << std::endl;
        } else {
            std::cerr << "Stop stream succeed" << std::endl;
        }
    }

private:
    PaStream* stream_ { nullptr };
    MicConf micConf_;
    OnAudioFrameCallback onAudioFrame_;
};

AudioSource::AudioSource(int path, const MicConf& conf, double latency)
    : impl_(new Impl(path, conf, latency))
{
}

AudioSource::~AudioSource()
{
    delete impl_;
}

void AudioSource::SetOnAudioFrame(const AudioSource::OnAudioFrameCallback& cb)
{
    return impl_->SetOnAudioFrame(cb);
}

bool AudioSource::StartCapture()
{
    return impl_->StartCapture();
}

void AudioSource::StopCapture()
{
    return impl_->StopCapture(true);
}
