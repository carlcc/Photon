//
// Created by carl on 20-4-3.
//

#include "AudioDestinationInfo.h"
#include "InitPortAudioHelper.h"
#include <iostream>
#include <portaudio.h>

static std::vector<double> GetSupportedStandardSampleRates(const PaStreamParameters* outputParameters)
{
    std::vector<double> result;

    static const double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };

    PaError err;

    for (int i = 0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(nullptr, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported) {
            result.push_back(standardSampleRates[i]);
        }
    }
    return result;
}

std::vector<AudioDestinationInfo> AudioDestinationInfo::QueryAllAudioDestinations()
{
    std::vector<AudioDestinationInfo> result;
    InitPortAudioHelper::Instance();

    auto numDevices = Pa_GetDeviceCount();

    if (numDevices < 0) {
        std::cerr << "ERROR: Pa_GetDeviceCount returned " << numDevices << std::endl;
        return result;
    }

    PaStreamParameters outputParameters;

    for (int i = 0; i < numDevices; ++i) {
        const auto* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxOutputChannels <= 0) {
            continue;
        }

        /* poll for standard sample rates */
        outputParameters.device = i;
        outputParameters.channelCount = 2;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        if (outputParameters.channelCount > 0) {
            auto sampleRates = GetSupportedStandardSampleRates(&outputParameters);
            if (!sampleRates.empty()) {
                std::vector<MicConf> micConfs;
                micConfs.reserve(sampleRates.size());
                for (auto sr : sampleRates) {
                    micConfs.push_back(MicConf { sr, uint32_t(outputParameters.channelCount) });
                }
                result.push_back({ i, deviceInfo->name, deviceInfo->defaultLowInputLatency, deviceInfo->defaultHighInputLatency, std::move(micConfs) });
            }
        }
    }

    return result;
}
