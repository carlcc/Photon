//
// Created by carl on 2020/4/5 0005.
//
#pragma once
#include "../../IVideoFrame.h"
#include "VideoSourceInfoWin32.h"
#include <SSBase/Str.h>
#include <functional>
#include <thread>

namespace DShow {
class Device;
}

class VideoSourceWin32 {
public:
    using OnDataCallback = std::function<void(const void* data)>;
    using OnFrameCallback = std::function<void(std::shared_ptr<IVideoFrame> frame)>;
    VideoSourceWin32(const VideoSourceInfoWin32& sourceInfo, const CameraConf& conf);
    ~VideoSourceWin32();

    void SetOnRawData(const OnDataCallback& onData);

    // NOTE: this callback will be called only callbackPixelFormat and onFrame are both set
    void SetOnFrame(const OnFrameCallback& onFrame);

    void SetCallbackPixelFormat(PixelFormat fmt);

    bool StartCapture();

    void StopCapture(bool waitUntilThreadStop);

    bool operator!() const
    {
        return !bool(*this);;
    }

    explicit operator bool() const;

private:
    void Close();

private:
    DShow::Device* device_ { nullptr };
    VideoFrameFormat frameFormat_;
    std::thread captureThread_ {};
    OnDataCallback onDataCallback_ { nullptr };
    OnFrameCallback onFrameCallback_ { nullptr };
    bool isRunning { false };
    PixelFormat outputFormat_ { PixelFormat::FMT_UNKNOWN };
    std::shared_ptr<YUV420PVideoFrame> yuvVideoFrame_ { nullptr };
};