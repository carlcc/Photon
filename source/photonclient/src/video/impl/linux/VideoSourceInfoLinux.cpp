//
// Created by carl on 20-4-2.
//

#include "VideoSourceInfoLinux.h"
#include "Video4Linux2Helper.h"
// https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/v4l2.html
#include <SSBase/Defer.h>
#include <SSIO/file/FileSystem.h>
#include <algorithm>
#include <fcntl.h>
#include <linux/v4l2-common.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

void MyPerror(const std::string& s)
{
    perror(s.c_str());
}

/**
 * Fixed framesizes for devices that don't support enumerating discrete values.
 *
 * The framesizes in this array are packed, the width encoded in the high word
 * and the height in the low word.
 * The array is terminated with a zero.
 */
static const uint32_t v4l2_framesizes[] = {
    /* 4:3 */
    160 << 16 | 120, 320 << 16 | 240, 480 << 16 | 320, 640 << 16 | 480,
    800 << 16 | 600, 1024 << 16 | 768, 1280 << 16 | 960, 1440 << 16 | 1050,
    1440 << 16 | 1080, 1600 << 16 | 1200,

    /* 16:9 */
    640 << 16 | 360, 960 << 16 | 540, 1280 << 16 | 720, 1600 << 16 | 900,
    1920 << 16 | 1080, 1920 << 16 | 1200, 2560 << 16 | 1440,
    3840 << 16 | 2160,

    /* 21:9 */
    2560 << 16 | 1080, 3440 << 16 | 1440, 5120 << 16 | 2160,

    /* tv */
    432 << 16 | 520, 480 << 16 | 320, 480 << 16 | 530, 486 << 16 | 440,
    576 << 16 | 310, 576 << 16 | 520, 576 << 16 | 570, 720 << 16 | 576,
    1024 << 16 | 576,

    0
};

static const uint32_t v4l2_framerates[] = {
    1 << 16 | 60,
    1 << 16 | 50,
    1 << 16 | 30,
    1 << 16 | 25,
    1 << 16 | 20,
    1 << 16 | 15,
    1 << 16 | 10,
    1 << 16 | 5,
    0
};

/**
 * Unpack two integer values from one
 *
 * @see v4l2_pack_tuple
 *
 * @param a pointer to integer a
 * @param b pointer to integer b
 * @param packed the packed integer
 */
static void v4l2_unpack_tuple(uint32_t* a, uint32_t* b, uint32_t packed)
{
    *a = packed >> 16u;
    *b = packed & 0xffffu;
}

std::vector<Resolution> ListSupportedFrameSizes(int fd, uint32_t v4l2Fmt)
{
    std::vector<Resolution> result;

    v4l2_frmsizeenum frmsize { 0 };
    frmsize.pixel_format = v4l2Fmt;
    frmsize.index = 0;

    ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize);

    switch (frmsize.type) {
    case V4L2_FRMSIZE_TYPE_DISCRETE:
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
            result.push_back({ frmsize.discrete.width, frmsize.discrete.height });
            frmsize.index++;
        }
        break;
    default:
        std::cerr << "Stepwise and Continuous framesizes are currently hardcoded" << std::endl;
        for (const uint32_t* packed = v4l2_framesizes; *packed; ++packed) {
            uint32_t width;
            uint32_t height;
            v4l2_unpack_tuple(&width, &height, *packed);
            result.push_back({ width, height });
        }
        break;
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    return result;
}

std::vector<FrameInterval> ListSupportedFrameIntervals(int fd, uint32_t v4l2Fmt, uint32_t width, uint32_t height)
{
    std::vector<FrameInterval> result;

    v4l2_frmivalenum frmival;
    frmival.pixel_format = v4l2Fmt;
    frmival.width = width;
    frmival.height = height;
    frmival.index = 0;

    ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival);

    switch (frmival.type) {
    case V4L2_FRMIVAL_TYPE_DISCRETE:
        while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {

            result.push_back({ frmival.discrete.numerator, frmival.discrete.denominator });

            frmival.index++;
        }
        break;
    default:
        std::cout << "Stepwise and Continuous framerates are currently hardcoded" << std::endl;

        for (const uint32_t* packed = v4l2_framerates; *packed; ++packed) {
            uint32_t num;
            uint32_t denom;
            v4l2_unpack_tuple(&num, &denom, *packed);
            result.push_back({ num, denom });
        }
        break;
    }

    return result;
}

std::vector<VideoSourceInfoLinux> VideoSourceInfoLinux::QueryAllVideoSources()
{
    std::vector<VideoSourceInfoLinux> result;
    auto videoFiles = ss::FileSystem::ListFileNames("/dev/", [](const ss::CharSequence& file) -> bool {
        return file.StartsWith("video");
    });
    for (auto& file : videoFiles) {
        ss::String path = "/dev/" + file;

        int fd = open(path.ToStdString().c_str(), O_RDWR);
        if (fd == -1) {
            // couldn't find capture device
            perror("Opening Video device ");
            continue;
        }
        OnScopeExit { close(fd); };

        // Query caps
        struct v4l2_capability caps = { 0 };
        if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &caps)) { // NOLINT
            MyPerror("Query Capabilities of " + path);
            continue;
        }
        if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE)) { // NOLINT
            continue;
        }

        // Query supported pixel formats
        std::vector<CameraConf> cameraConfs;
        v4l2_fmtdesc fmtDesc = { 0 };
        fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        for (; ioctl(fd, VIDIOC_ENUM_FMT, &fmtDesc) == 0; fmtDesc.index++) { // NOLINT
            auto fmt = V4l2PixelFormatToPhotonPixelFormat(fmtDesc.pixelformat);
            if (fmt == PixelFormat::FMT_UNKNOWN) {
                continue;
            }

            auto frameSizes = ListSupportedFrameSizes(fd, fmtDesc.pixelformat);
            for (auto& size : frameSizes) {
                auto frameIntervals = ListSupportedFrameIntervals(fd, fmtDesc.pixelformat, size.width, size.height);
                for (auto interval : frameIntervals) {
                    cameraConfs.push_back({{fmt, size}, interval});
                }
            }
        }
        if (cameraConfs.empty()) {
            continue;
        }

        result.push_back({ std::move(path), ss::String((const char*)caps.card), std::move(cameraConfs) });
    }
    return result;
}
