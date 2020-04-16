//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "../../IVideoFrame.h"
#include "VideoSourceInfoMac.h"
#include <thread>

class VideoSourceMac {
public:
    using OnDataCallback = std::function<void(const void* data)>;
    using OnFrameCallback = std::function<void(std::shared_ptr<IVideoFrame> frame)>;
    VideoSourceMac(const VideoSourceInfoMac& sourceInfo, const CameraConf& conf);
    ~VideoSourceMac();

    void SetOnRawData(const OnDataCallback& onData);

    // NOTE: this callback will be called only callbackPixelFormat and onFrame are both set
    void SetOnFrame(const OnFrameCallback& onFrame);

    void SetCallbackPixelFormat(PixelFormat fmt);

    bool StartCapture();

    void StopCapture(bool waitUntilThreadStop);

    bool operator!() const
    {
        return impl_ == nullptr;
    }

    explicit operator bool() const
    {
        return impl_ != nullptr;
    }

    class Impl;
private:
    void Close();

private:
    Impl* impl_;
};
