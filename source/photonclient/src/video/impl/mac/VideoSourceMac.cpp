//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "VideoSourceMac.h"

VideoSourceMac::VideoSourceMac(const VideoSourceInfoMac& sourceInfo, const CameraConf& conf)
{
}

VideoSourceMac::~VideoSourceMac()
{
}

void VideoSourceMac::SetOnRawData(const VideoSourceMac::OnDataCallback& onData)
{
}

void VideoSourceMac::SetOnFrame(const std::function<void(std::shared_ptr<IVideoFrame>)>& onFrame)
{
}

void VideoSourceMac::SetCallbackPixelFormat(PixelFormat fmt)
{
}

bool VideoSourceMac::StartCapture()
{
    return false;
}

void VideoSourceMac::StopCapture(bool waitUntilThreadStop)
{
}

void VideoSourceMac::Close()
{
}
