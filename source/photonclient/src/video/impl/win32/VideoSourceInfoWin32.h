//
// Created by carl on 2020/4/4 0004.
//
#pragma once

#include "../../VideoFrameFormat.h"
#include <SSBase/Str.h>
#include <string>
#include <vector>

class VideoSourceInfoWin32 {
public:
    static std::vector<VideoSourceInfoWin32> QueryAllVideoSources();

    ss::String path {};
    ss::String cardName {};
    std::vector<CameraConf> supportedCameraConfs;
};
