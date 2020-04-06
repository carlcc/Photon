//
// Created by carl on 20-4-2.
//

#pragma once

#if defined(PHOTON_PLATFORM_LINUX)

#include "impl/linux/VideoSourceInfoLinux.h"
using VideoSourceInfo = VideoSourceInfoLinux;

#elif defined(PHOTON_PLATFORM_WIN)

#include "impl/win32/VideoSourceInfoWin32.h"
using VideoSourceInfo = VideoSourceInfoWin32;

#elif defined(PHOTON_PLATFORM_APPLE)

#error NYI

#endif
