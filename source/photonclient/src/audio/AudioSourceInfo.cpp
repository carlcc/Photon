//
// Created by carl on 20-4-3.
//

#include "AudioSourceInfo.h"
#include "InitPortAudioHelper.h"
#include <iostream>
#include <portaudio.h>

static std::vector<double> GetSupportedStandardSampleRates(const PaStreamParameters* inputParameters)
{
    std::vector<double> result;

    static const double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };

    PaError err;

    for (int i = 0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(inputParameters, nullptr, standardSampleRates[i]);
        if (err == paFormatIsSupported) {
            result.push_back(standardSampleRates[i]);
        }
    }
    return result;
}

std::vector<AudioSourceInfo> AudioSourceInfo::QueryAllAudioSources()
{
    std::vector<AudioSourceInfo> result;
    InitPortAudioHelper::Instance();

    auto numDevices = Pa_GetDeviceCount();

    if (numDevices < 0) {
        std::cerr << "ERROR: Pa_GetDeviceCount returned " << numDevices << std::endl;
        return result;
    }

    PaStreamParameters inputParameters;

    for (int i = 0; i < numDevices; ++i) {
        const auto* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxInputChannels <= 0) {
            continue;
        }

        std::vector<MicConf> micConfs;
        for (int channelCount = 2; channelCount > 0; --channelCount) {
            /* poll for standard sample rates */
            inputParameters.device = i;
            inputParameters.channelCount = channelCount;
            inputParameters.sampleFormat = paInt16;
            inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
            inputParameters.hostApiSpecificStreamInfo = nullptr;
            auto sampleRates = GetSupportedStandardSampleRates(&inputParameters);

            if (!sampleRates.empty()) {
                micConfs.reserve(micConfs.size() + sampleRates.size());
                for (auto sr : sampleRates) {
                    micConfs.push_back(MicConf { sr, uint32_t(inputParameters.channelCount) });
                }
            }
        }
        if (!micConfs.empty()) {
            result.push_back({ i, deviceInfo->name, deviceInfo->defaultLowInputLatency, deviceInfo->defaultHighInputLatency, std::move(micConfs) });
        }
    }

    return result;
}
