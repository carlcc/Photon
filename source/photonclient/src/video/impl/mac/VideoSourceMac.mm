//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "VideoSourceMac.h"
#include "AVFoundationHelper.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <SSBase/Assert.h>
#include <SSBase/Defer.h>
#include <libyuv.h>

@interface VideoSourceMacImpl : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property VideoSourceMac::Impl* cppImpl_;

- (void)captureOutput:(AVCaptureOutput*)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection*)connection;

@end

class VideoSourceMac::Impl {
public:
    Impl(const VideoSourceInfoMac& sourceInfo, const CameraConf& conf)
    {
        NSArray* cameras = GetAllCameras();
        AVCaptureDevice* camera = nullptr;
        for (AVCaptureDevice* device in cameras) {

            ss::String name([device.localizedName UTF8String]);
            ss::String path([device.uniqueID UTF8String]);

            if (sourceInfo.cardName == name && sourceInfo.path == path) {
                camera = device;
                break;
            }
        }

        if (camera == nullptr) {
            Close();
            return;
        }
        ocManagedImpl_ = [[VideoSourceMacImpl alloc] init];
        ocManagedImpl_.cppImpl_ = this;

        frameFormat_ = conf.frameFormat;
        camera_ = camera;
    }

    ~Impl()
    {
        // TODO: seems unsafe, we must ensure ocManagedImpl_::captureOutput will
        // no longer invoke this.OnFrameData, then destroy this object. But
        // this.StopCapture can not ensure this by now.
        StopCapture(true);
        Close();
    }

    void Close()
    {
        if (session_ != nullptr) {
            [session_ release];
            session_ = nullptr;
        }
        if (camera_ != nullptr) {
            [camera_ release];
            camera_ = nullptr;
        }
        if (ocManagedImpl_ != nullptr) {
            [ocManagedImpl_ release];
            ocManagedImpl_ = nullptr;
        }
    }

    void SetOnRawData(const OnDataCallback& onData)
    {
        onDataCallback_ = onData;
    }

    // NOTE: this callback will be called only callbackPixelFormat and onFrame are both set
    void SetOnFrame(const OnFrameCallback& onFrame)
    {
        onFrameCallback_ = onFrame;
    }

    void SetCallbackPixelFormat(PixelFormat fmt)
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

    bool StartCapture()
    {
        SSASSERT(camera_ != nullptr);
        
        AVCaptureSession* session = [[AVCaptureSession alloc] init];
        session.sessionPreset = AVCaptureSessionPresetMedium;

        NSError* error = nil;

        AVCaptureDevice* captureDevice = camera_;
        [camera_ lockForConfiguration:&error];
        [camera_ setActiveVideoMinFrameDuration:CMTimeMake(1,4)];
        [camera_ setActiveVideoMaxFrameDuration:CMTimeMake(1,4)];
        [camera_ unlockForConfiguration];
        AVCaptureDeviceInput* videoInput = [[AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&error] autorelease];

        AVCaptureVideoDataOutput* videoOutput = [[[AVCaptureVideoDataOutput alloc] init] autorelease];

        if (error) {
            NSLog(@"Error getting video input device: %@", error.description);
            return false;
        }
        [session beginConfiguration];
        // 添加输入
        if ([session canAddInput:videoInput]) {
            [session addInput:videoInput];
            videoInput = videoInput;
        }
        // 添加输出
        if ([session canAddOutput:videoOutput]) {
            [session addOutput:videoOutput];
            videoOutput = videoOutput;
        }
        [session commitConfiguration];

        dispatch_queue_t queue = dispatch_queue_create("myQueue", NULL);
        [videoOutput setSampleBufferDelegate:ocManagedImpl_ queue:queue];
        dispatch_release(queue);

        videoOutput.videoSettings = [NSDictionary dictionaryWithObject:
                                                      [NSNumber numberWithInt:PhotonPixelFormatToAVFoundationPixelFormat(frameFormat_.pixelFormat)]
                                                                forKey:(id)kCVPixelBufferPixelFormatTypeKey];

        // TODO: set framerate and resolution
        // videoOutput.minFrameDuration = CMTimeMake(1, 15);
        // videoOutput.maxFrameDuration = CMTimeMake(1, 15);

        [session startRunning];

        session_ = session;
        return true;
    }

    void StopCapture(bool waitUntilThreadStop)
    {
        if (session_ != nullptr) {
            [session_ stopRunning];
        }
    }

    bool operator!() const
    {
        return camera_ == nullptr;
    }

    explicit operator bool() const
    {
        return camera_ != nullptr;
    }

    void OnFrameData(const uint8_t* buffer, size_t bytesPerRow, size_t width, size_t height, size_t bufferSize)
    {
        if (onDataCallback_) {
            onDataCallback_(buffer);
        }
        if (onFrameCallback_ && outputFormat_ != PixelFormat::FMT_UNKNOWN) {
            if (frameFormat_.pixelFormat == PixelFormat::FMT_YUYV422) {
                libyuv::YUY2ToI420(buffer, bytesPerRow,
                    yuvVideoFrame_->Y(), yuvVideoFrame_->YStride(),
                    yuvVideoFrame_->U(), yuvVideoFrame_->VStride(),
                    yuvVideoFrame_->V(), yuvVideoFrame_->VStride(),
                    width, height);
            } else {
                std::cout << "Pixel format " << GetPixelFormatName(frameFormat_.pixelFormat) << " is not implemented" << std::endl;
                abort();
            }
            onFrameCallback_(yuvVideoFrame_);
        }
    }

private:
    VideoSourceMacImpl* ocManagedImpl_ { nullptr };

    AVCaptureDevice* camera_ { nullptr };
    AVCaptureSession* session_ { nullptr };
    VideoFrameFormat frameFormat_ {};
    OnDataCallback onDataCallback_ { nullptr };
    OnFrameCallback onFrameCallback_ { nullptr };
    PixelFormat outputFormat_ { PixelFormat::FMT_UNKNOWN };
    std::shared_ptr<YUV420PVideoFrame> yuvVideoFrame_ { nullptr };
};

@implementation VideoSourceMacImpl

- (void)captureOutput:(AVCaptureOutput*)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection*)connection
{
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    CVPixelBufferLockBaseAddress(imageBuffer, 0);

    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    void* baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    size_t bufferSize = CVPixelBufferGetDataSize(imageBuffer);

    if (self.cppImpl_ != nullptr) {
        self.cppImpl_->OnFrameData((uint8_t*)baseAddress, bytesPerRow, width, height, bufferSize);
    }

    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
}

@end

VideoSourceMac::VideoSourceMac(const VideoSourceInfoMac& sourceInfo, const CameraConf& conf)
    : impl_(new Impl(sourceInfo, conf))
{
    if (!(*impl_)) {
        Close();
    }
}

VideoSourceMac::~VideoSourceMac()
{
    StopCapture(true);
    Close();
}

void VideoSourceMac::SetOnRawData(const VideoSourceMac::OnDataCallback& onData)
{
    impl_->SetOnRawData(onData);
}

void VideoSourceMac::SetOnFrame(const std::function<void(std::shared_ptr<IVideoFrame>)>& onFrame)
{
    impl_->SetOnFrame(onFrame);
}

void VideoSourceMac::SetCallbackPixelFormat(PixelFormat fmt)
{
    return impl_->SetCallbackPixelFormat(fmt);
}

bool VideoSourceMac::StartCapture()
{
    return impl_->StartCapture();
}

void VideoSourceMac::StopCapture(bool waitUntilThreadStop)
{
    impl_->StopCapture(waitUntilThreadStop);
}

void VideoSourceMac::Close()
{
    if (impl_ != nullptr) {
        impl_->Close();
        delete impl_;
        impl_ = nullptr;
    }
}
