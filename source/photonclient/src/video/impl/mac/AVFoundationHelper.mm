//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "AVFoundationHelper.h"
#import <AVFoundation/AVFoundation.h>

PixelFormat AVFoundationPixelFormatToPhotonPixelFormat(uint32_t format)
{
    switch (format) {
    case kCVPixelFormatType_422YpCbCr8_yuvs:
        return PixelFormat::FMT_YUYV422;
    default:
        return PixelFormat::FMT_UNKNOWN;
    }
}

uint32_t PhotonPixelFormatToAVFoundationPixelFormat(PixelFormat format)
{
    switch (format) {
    case PixelFormat::FMT_YUYV422:
        return kCVPixelFormatType_422YpCbCr8_yuvs;

    default:
        return -1;
    }
}
