//
// Created by carl on 20-4-3.
//

#include "VideoSourceLinux.h"
#include "Video4Linux2Helper.h"
#include <SSBase/Defer.h>
#include <fcntl.h>
#include <iostream>
#include <libyuv.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

VideoSourceLinux::VideoSourceLinux(const VideoSourceInfoLinux& sourceInfo, const CameraConf& conf)
{
    frameFormat_ = conf.frameFormat;
    fd_ = open(sourceInfo.path.ToStdString().c_str(), O_RDWR);
    if (fd_ == -1) {
        perror(sourceInfo.path.ToStdString().c_str());
        return;
    }

    ss::Defer defer = [&]() {
        Close();
    };

    uint32_t v4l2PixFmt = PhotonPixelFormatToV4l2PixelFormat(frameFormat_.pixelFormat);
    if (v4l2PixFmt == -1) {
        std::cerr << "Unkown pixel format: " << frameFormat_.pixelFormat << std::endl;
        return;
    }

    v4l2_format v4l2Fmt = { 0 };
    v4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2Fmt.fmt.pix.width = frameFormat_.resolution.width;
    v4l2Fmt.fmt.pix.height = frameFormat_.resolution.height;
    v4l2Fmt.fmt.pix.pixelformat = v4l2PixFmt;
    v4l2Fmt.fmt.pix.field = V4L2_FIELD_NONE;
    v4l2Fmt.fmt.pix.bytesperline = 0;

    if (-1 == ioctl(fd_, VIDIOC_S_FMT, &v4l2Fmt)) {
        perror("Setting Pixel Format");
        return;
    }

    v4l2_streamparm v4l2Interval = { 0 };
    v4l2Interval.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2Interval.parm.capture.timeperframe.numerator = conf.frameInterval.numerator;
    v4l2Interval.parm.capture.timeperframe.denominator = conf.frameInterval.denominator;
    if (-1 == ioctl(fd_, VIDIOC_S_PARM, &v4l2Interval)) {
        perror("Setting frame rate");
        return;
    }

    defer.Cancel();
}

VideoSourceLinux::~VideoSourceLinux()
{
    StopCapture(true);
    Close();
}

void VideoSourceLinux::SetOnRawData(const VideoSourceLinux::OnDataCallback& onData)
{
    onDataCallback_ = onData;
}

void VideoSourceLinux::SetOnFrame(const OnFrameCallback& onFrame)
{
    onFrameCallback_ = onFrame;
}

bool VideoSourceLinux::StartCapture()
{
    if (isRunning) {
        std::cerr << "Already running" << std::endl;
        return true;
    }

    isRunning = true;
    captureThread_ = std::thread([this]() {
        ::ss::Defer deferStop = [this]() { isRunning = false; };

        struct v4l2_requestbuffers req = { 0 };
        req.count = 1;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(fd_, VIDIOC_REQBUFS, &req)) {
            perror("Requesting Buffer");
            return;
        }

        struct v4l2_buffer buf = { 0 };
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;
        if (-1 == ioctl(fd_, VIDIOC_QUERYBUF, &buf)) {
            perror("Querying Buffer");
            return;
        }

        void* buffer = mmap(nullptr, buf.length, PROT_READ, MAP_SHARED, fd_, buf.m.offset);
        if (buffer == MAP_FAILED) {
            perror("mmap Buffer");
            return;
        }
        ::ss::Defer defer = [buffer, &buf] { munmap(buffer, buf.length); };

        if (-1 == ioctl(fd_, VIDIOC_STREAMON, &buf.type)) {
            perror("Start Capture");
            return;
        }
        ::ss::Defer deferStreamOff = [&buf, this]() { ioctl(fd_, VIDIOC_STREAMOFF, &buf.type); };

        while (isRunning) {

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd_, &fds);
            struct timeval tv = { 0 };
            tv.tv_sec = 0;
            tv.tv_usec = 50000;

            if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
                perror("Could not queue buffer, VIDIOC_QBUF");
                return;
            }

            int r = select(fd_ + 1, &fds, NULL, NULL, &tv);
            if (-1 == r) {
                perror("Waiting for Frame");
                return;
            }

            if (-1 == ioctl(fd_, VIDIOC_DQBUF, &buf)) {
                perror("Retrieving Frame");
                return;
            }

            if (onDataCallback_) {
                onDataCallback_(buffer);
            }
            if (onFrameCallback_ && outputFormat_ != PixelFormat::FMT_UNKNOWN) {
                if (frameFormat_.pixelFormat == PixelFormat::FMT_YUYV422) {
                    libyuv::YUY2ToI420(static_cast<const uint8_t*>(buffer), frameFormat_.resolution.width * 2,
                        yuvVideoFrame_->Y(), yuvVideoFrame_->YStride(),
                        yuvVideoFrame_->U(), yuvVideoFrame_->VStride(),
                        yuvVideoFrame_->V(), yuvVideoFrame_->VStride(),
                        frameFormat_.resolution.width, frameFormat_.resolution.height);
                } else {
                    std::cout << "Pixel format " << GetPixelFormatName(frameFormat_.pixelFormat) << " is not implemented" << std::endl;
                    abort();
                }
                onFrameCallback_(yuvVideoFrame_);
            }
        }
    });
    return true;
}

void VideoSourceLinux::StopCapture(bool waitUntilThreadStop)
{
    isRunning = false;
    if (waitUntilThreadStop) {
        if (captureThread_.joinable()) {
            captureThread_.join();
        }
    }
}

void VideoSourceLinux::Close()
{
    if (fd_) {
        close(fd_);
        fd_ = -1;
    }
}

void VideoSourceLinux::SetCallbackPixelFormat(PixelFormat fmt)
{
    if (fmt != FMT_YUV420P) {
        std::cerr << "VideoSource auto convert to non yuv420p is no supported now" << std::endl;
        return;
    }
    if (fmt == outputFormat_) {
        return;
    }
    outputFormat_ = fmt;
    yuvVideoFrame_ = std::make_shared<YUV420PVideoFrame>(frameFormat_.resolution.width, frameFormat_.resolution.height);
}
