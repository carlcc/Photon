//
// Created by carl on 20-4-3.
//

#include "Video4Linux2Helper.h"
#include "../../VideoFrameFormat.h"
#include <linux/v4l2-common.h>
#include <linux/videodev2.h>

// https://www.kernel.org/doc/html/v4.14/media/uapi/v4l/pixfmt-yuv420.html
PixelFormat V4l2PixelFormatToPhotonPixelFormat(uint32_t format)
{
    switch (format) {
    case V4L2_PIX_FMT_YUYV:
        return PixelFormat ::FMT_YUYV422;
    case V4L2_PIX_FMT_YVYU:
        return PixelFormat ::FMT_YVYU422;
        // case V4L2_PIX_FMT_UYVY:
        //     return VIDEO_FORMAT_UYVY;
        // case V4L2_PIX_FMT_NV12:
        //     return VIDEO_FORMAT_NV12;
    case V4L2_PIX_FMT_YUV420:
        return PixelFormat::FMT_YUV420P;
    case V4L2_PIX_FMT_RGB24:
        return PixelFormat::FMT_RGB888;
        //    case V4L2_PIX_FMT_YVU420:
        //        return VIDEO_FORMAT_I420;
        //#ifdef V4L2_PIX_FMT_XBGR32
        //    case V4L2_PIX_FMT_XBGR32:
        //        return VIDEO_FORMAT_BGRX;
        //#endif
        //#ifdef V4L2_PIX_FMT_ABGR32
        //    case V4L2_PIX_FMT_ABGR32:
        //        return VIDEO_FORMAT_BGRA;
        //#endif
        //    case V4L2_PIX_FMT_BGR24:
        //        return VIDEO_FORMAT_BGR3;
    default:
        return PixelFormat::FMT_UNKNOWN;
    }
}

uint32_t PhotonPixelFormatToV4l2PixelFormat(PixelFormat format)
{
    switch (format) {
    case PixelFormat::FMT_RGB888:
        return V4L2_PIX_FMT_RGB24;
    case PixelFormat::FMT_YUYV422:
        return V4L2_PIX_FMT_YUYV;
    case PixelFormat::FMT_YVYU422:
        return V4L2_PIX_FMT_YVYU;
    case PixelFormat::FMT_YUV420P:
        return V4L2_PIX_FMT_YUV420;
    case PixelFormat::FMT_UNKNOWN:
    default:
        return -1;
    }
}