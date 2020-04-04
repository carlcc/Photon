//
// Created by carl on 20-4-3.
//

#pragma once

#include "../../VideoFrameFormat.h"
#include <cstdint>

PixelFormat V4l2PixelFormatToPhotonPixelFormat(uint32_t format);

uint32_t PhotonPixelFormatToV4l2PixelFormat(PixelFormat format);
