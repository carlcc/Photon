//
// Created by carl on 20-4-2.
//

#pragma once
#include <cstdint>

enum PixelFormat : uint8_t {
    FMT_UNKNOWN,
    FMT_RGB888,
    FMT_YUYV422, // https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt-yuyv.html
    FMT_YVYU422,
    FMT_YUV420P,
};

inline const char* GetPixelFormatName(PixelFormat fmt)
{
    switch (fmt) {
    case PixelFormat::FMT_RGB888:
        return "RGB888";
    case PixelFormat::FMT_YUYV422:
        return "YUYV422";
    case PixelFormat::FMT_YVYU422:
        return "YVYU422";
    case PixelFormat::FMT_YUV420P:
        return "YUV420P";
    default:
    case PixelFormat::FMT_UNKNOWN:
        return "UNKNOWN";
    }
}

struct Resolution {
    uint32_t width;
    uint32_t height;
};
inline bool operator==(const Resolution& a, const Resolution& b)
{
    return a.width == b.width && a.height == b.height;
}
inline bool operator<(const Resolution& a, const Resolution& b)
{
    return a.width * a.height < b.width * b.height;
}

struct VideoFrameFormat {
    PixelFormat pixelFormat;
    Resolution resolution;
};
inline bool operator==(const VideoFrameFormat& a, const VideoFrameFormat& b)
{
    return a.pixelFormat == b.pixelFormat && a.resolution == b.resolution;
}

struct FrameInterval {
    uint32_t numerator;
    uint32_t denominator;
};
inline bool operator==(const FrameInterval& a, const FrameInterval& b)
{
    uint64_t p1 = uint64_t(1) * a.numerator * b.denominator;
    uint64_t p2 = uint64_t(1) * a.denominator * b.numerator;
    return p1 == p2;
}

struct CameraConf {
    VideoFrameFormat frameFormat;
    FrameInterval frameInterval;
};
inline bool operator==(const CameraConf& a, const CameraConf& b)
{
    return a.frameFormat == b.frameFormat && a.frameInterval == b.frameInterval;
}