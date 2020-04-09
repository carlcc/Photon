//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "../../VideoFrameFormat.h"
#include <SSBase/Str.h>
#include <string>
#include <vector>

class VideoSourceInfoMac {
public:
    static std::vector<VideoSourceInfoMac> QueryAllVideoSources();

    ss::String path;
    ss::String cardName;
    std::vector<CameraConf> supportedCameraConfs;
};

