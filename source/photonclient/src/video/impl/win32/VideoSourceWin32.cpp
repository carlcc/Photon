//
// Created by carl on 2020/4/5 0005.
//

#include "VideoSourceWin32.h"
#include "DShowHelper.h"
#include <SSBase/Assert.h>
#include <dshowcapture.hpp>
#include <iostream>
#include <libyuv.h>

VideoSourceWin32::VideoSourceWin32(const VideoSourceInfoWin32& sourceInfo, const CameraConf& conf)
{
    DShow::VideoFormat pixelFormat = PhotonPixelFormatToDShowPixelFormat(conf.frameFormat.pixelFormat);
    if (pixelFormat == DShow::VideoFormat::Unknown) {
        std::cerr << "Unknown pixel format" << std::endl;
        return;
    }

    device_ = new DShow::Device(DShow::InitGraph::True);
    if (!device_->Valid()) {
        Close();
        return;
    }

    frameFormat_ = conf.frameFormat;

    DShow::VideoConfig videoConfig;
    videoConfig.useDefaultConfig = false;
    videoConfig.cx = conf.frameFormat.resolution.width;
    videoConfig.cy_abs = conf.frameFormat.resolution.height;
    videoConfig.cy_flip = false;
    videoConfig.frameInterval = conf.frameInterval.numerator;
    videoConfig.format = pixelFormat;
    videoConfig.name = sourceInfo.cardName.ToStdWString();
    videoConfig.path = sourceInfo.path.ToStdWString();
    videoConfig.callback = [this](const DShow::VideoConfig& config, unsigned char* data,
                               size_t size, long long startTime, long long stopTime,
                               long rotation) {
        if (onDataCallback_) {
            onDataCallback_(data);
        }
        if (onFrameCallback_ && outputFormat_ != PixelFormat::FMT_UNKNOWN) {
            if (frameFormat_.pixelFormat == PixelFormat::FMT_YUYV422) {
                // yuvVideoFrame_->width = config.cx;
                // yuvVideoFrame_->height = config.cy_abs;
                libyuv::YUY2ToI420((const uint8_t*)data, frameFormat_.resolution.width * 2,
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
    };
    if (!device_->SetVideoConfig(&videoConfig)) {
        std::cerr << "Set Video Config failed" << std::endl;
        Close();
    }
    if (!device_->ConnectFilters()) {
        std::cerr << "Connect filters failed" << std::endl;
        Close();
    }
}

VideoSourceWin32::~VideoSourceWin32()
{
    StopCapture(true);
    Close();
}

void VideoSourceWin32::SetOnRawData(const VideoSourceWin32::OnDataCallback& onData)
{
    onDataCallback_ = onData;
}

void VideoSourceWin32::SetOnFrame(const VideoSourceWin32::OnFrameCallback& onFrame)
{
    onFrameCallback_ = onFrame;
}

void VideoSourceWin32::SetCallbackPixelFormat(PixelFormat fmt)
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

bool VideoSourceWin32::StartCapture()
{
    if (!(*this)) {
        return false;
    }
    return DShow::Result::Success == device_->Start();
}

void VideoSourceWin32::StopCapture(bool waitUntilThreadStop)
{
    if (!(*this)) {
        return;
    }
    device_->Stop();
}

void VideoSourceWin32::Close()
{
    if (device_ != nullptr) {
        delete device_;
        device_ = nullptr;
    }
}

VideoSourceWin32::operator bool() const
{
    return device_ != nullptr && device_->Valid();
}
