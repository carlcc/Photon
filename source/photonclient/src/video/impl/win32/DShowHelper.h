//
// Created by carl on 2020/4/6 0006.
//
#pragma once

#include "../../VideoFrameFormat.h"
#include <dshowcapture.hpp>
#include <cstdint>

PixelFormat DShowPixelFormatToPhotonPixelFormat(DShow::VideoFormat format);

DShow::VideoFormat PhotonPixelFormatToDShowPixelFormat(PixelFormat format);