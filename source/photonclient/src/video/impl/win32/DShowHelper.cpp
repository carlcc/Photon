//
// Created by carl on 2020/4/6 0006.
//

#include "DShowHelper.h"

PixelFormat DShowPixelFormatToPhotonPixelFormat(DShow::VideoFormat format)
{
    switch (format) {
    /* raw formats */
    // case DShow::VideoFormat::ARGB:
    // case DShow::VideoFormat::XRGB:

    /* planar YUV formats */
    case DShow::VideoFormat::I420:
        return PixelFormat ::FMT_YUV420P;
    // case DShow::VideoFormat::NV12:
    // case DShow::VideoFormat::YV12:
    // case DShow::VideoFormat::Y800:

    /* packed YUV formats */
    case DShow::VideoFormat::YVYU:
        return PixelFormat::FMT_YVYU422;
    case DShow::VideoFormat::YUY2:
        return PixelFormat::FMT_YUYV422;
    // case DShow::VideoFormat::UYVY:
    // case DShow::VideoFormat::HDYC:
    default:
        return PixelFormat::FMT_UNKNOWN;
    }
}

DShow::VideoFormat PhotonPixelFormatToDShowPixelFormat(PixelFormat format)
{
    switch (format) {
    case PixelFormat::FMT_YUYV422:
        return DShow::VideoFormat::YUY2;
    case PixelFormat::FMT_YVYU422:
        return DShow::VideoFormat::YVYU;
    case PixelFormat::FMT_YUV420P:
        return DShow::VideoFormat::I420;

    case PixelFormat::FMT_UNKNOWN:
    case PixelFormat::FMT_RGB888:
    default:
        return DShow::VideoFormat::Unknown;
    }
}