//
// Created by carl on 20-4-3.
//

#pragma once

#include "AudioFrameFormat.h"
#include <string>
#include <vector>

class AudioSourceInfo {
public:
    static std::vector<AudioSourceInfo> QueryAllVideoSources();

    int path;
    std::string cardName;
    double lowLatency;
    double highLatency;
    std::vector<MicConf> supportedMicConfs;
};
