//
// Created by carl on 20-4-2.
//
#pragma once
#include "../../VideoFrameFormat.h"
#include <SSBase/Str.h>
#include <string>
#include <vector>

class VideoSourceInfoLinux {
public:
    static std::vector<VideoSourceInfoLinux> QueryAllVideoSources();

    ss::String path;
    ss::String cardName;
    std::vector<CameraConf> supportedCameraConfs;
};
