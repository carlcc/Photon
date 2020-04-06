//
// Created by carl on 20-4-2.
//
#pragma once
#include "VideoSourceInfo.h"

#if defined(PHOTON_PLATFORM_LINUX)

#include "impl/linux/VideoSourceLinux.h"
using VideoSource = VideoSourceLinux;

#elif defined(PHOTON_PLATFORM_WIN)

#include "impl/win32/VideoSourceWin32.h"
using VideoSource = VideoSourceWin32;

#elif defined(PHOTON_PLATFORM_APPLE)

#error NYI

#endif
