//
// Created by carl on 20-4-3.
//

#include "AudioSourceInfo.h"
#include <iostream>
#include <portaudio.h>

struct InitPortAudioHelper {
    InitPortAudioHelper()
    {
        auto error = Pa_Initialize();
        if (paNoError != error) {
            std::cerr << "Initilize port audio failed" << std::endl;
            sIsInitialized_ = false;
            return;
        }

        sIsInitialized_ = true;
    }
    ~InitPortAudioHelper()
    {
        if (sIsInitialized_) {
            sIsInitialized_ = false;
            Pa_Terminate();
        }
    }

    static bool sIsInitialized_;
};

bool InitPortAudioHelper::sIsInitialized_ { false };

std::vector<double> GetSupportedStandardSampleRates(const PaStreamParameters* inputParameters)
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

static void PrintSupportedStandardSampleRates(
    const PaStreamParameters* inputParameters,
    const PaStreamParameters* outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int i, printCount;
    PaError err;

    printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported) {
            if (printCount == 0) {
                printf("\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            } else if (printCount == 4) {
                printf(",\n\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            } else {
                printf(", %8.2f", standardSampleRates[i]);
                ++printCount;
            }
        }
    }
    if (!printCount)
        printf("None\n");
    else
        printf("\n");
}

std::vector<AudioSourceInfo> AudioSourceInfo::QueryAllVideoSources()
{
    std::vector<AudioSourceInfo> result;
    static InitPortAudioHelper unusedInitPortAudioHelper_;

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

        /* poll for standard sample rates */
        inputParameters.device = i;
        inputParameters.channelCount = 2;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        if (inputParameters.channelCount > 0) {
            auto sampleRates = GetSupportedStandardSampleRates(&inputParameters);
            if (!sampleRates.empty()) {
                std::vector<MicConf> micConfs;
                micConfs.reserve(sampleRates.size());
                for (auto sr : sampleRates) {
                    micConfs.push_back(MicConf { sr, uint32_t(inputParameters.channelCount) });
                }
                result.push_back({ i, deviceInfo->name, deviceInfo->defaultLowInputLatency, deviceInfo->defaultHighInputLatency, std::move(micConfs) });
            }
        }
    }

    return result;
}
