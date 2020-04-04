//
// Created by carl on 20-4-3.
//
#pragma once
#include "../../IVideoFrame.h"
#include "VideoSourceInfoLinux.h"
#include <functional>
#include <thread>

class VideoSourceLinux {
public:
    using OnDataCallback = std::function<void(const void* data)>;
    using OnFrameCallback = std::function<void(std::shared_ptr<IVideoFrame> frame)>;
    VideoSourceLinux(const std::string& path, const CameraConf& conf);
    ~VideoSourceLinux();

    void SetOnRawData(const OnDataCallback& onData);

    // NOTE: this callback will be called only callbackPixelFormat and onFrame are both set
    void SetOnFrame(const OnFrameCallback& onFrame);

    void SetCallbackPixelFormat(PixelFormat fmt);

    bool StartCapture();

    void StopCapture(bool waitUntilThreadStop);

    bool operator!() const
    {
        return fd_ == -1;
    }

    explicit operator bool() const
    {
        return fd_ != -1;
    }

private:
    void Close();

private:
    int fd_ { -1 };
    VideoFrameFormat frameFormat_;
    std::thread captureThread_ {};
    OnDataCallback onDataCallback_ { nullptr };
    OnFrameCallback onFrameCallback_ { nullptr };
    bool isRunning { false };
    PixelFormat outputFormat_ { PixelFormat::FMT_UNKNOWN };
    std::shared_ptr<YUV420PVideoFrame> yuvVideoFrame_ { nullptr };
};
