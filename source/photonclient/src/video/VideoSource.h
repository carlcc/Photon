//
// Created by carl on 20-4-2.
//
#pragma once
#include "VideoSourceInfo.h"

#if defined(PHOTON_PLATFORM_LINUX)

#include "impl/linux/VideoSourceLinux.h"
using VideoSource = VideoSourceLinux;

#elif defined(PHOTON_PLATFORM_WIN)

#error NYI

#elif defined(PHOTON_PLATFORM_APPLE)

#error NYI

#endif
