//
// Created by carl on 20-4-2.
//
#pragma once
#include "../../FrameFormat.h"
#include <string>
#include <vector>

class VideoSourceInfoLinux {
public:
    static std::vector<VideoSourceInfoLinux> QueryAllVideoSources();

    std::string path;
    std::string cardName;
    std::vector<CameraConf> supportedCameraConfs;
};
