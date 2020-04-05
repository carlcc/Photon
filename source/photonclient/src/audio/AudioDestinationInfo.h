//
// Created by carl on 20-4-3.
//

#pragma once

#include "AudioFrameFormat.h"
#include <string>
#include <vector>

class AudioDestinationInfo {
public:
    static std::vector<AudioDestinationInfo> QueryAllAudioDestinations();

    int path;
    std::string cardName;
    double lowLatency;
    double highLatency;
    std::vector<MicConf> supportedMicConfs;
};
