//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "../../VideoFrameFormat.h"
#include <cstdint>
#import <CoreFoundation/CoreFoundation.h>

PixelFormat AVFoundationPixelFormatToPhotonPixelFormat(uint32_t format);

uint32_t PhotonPixelFormatToAVFoundationPixelFormat(PixelFormat format);

int CanAccessVideo();

NSArray* GetAllCameras();
