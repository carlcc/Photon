//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "VideoSourceInfoMac.h"
#include "AVFoundationHelper.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <SSBase/Str.h>

std::vector<VideoSourceInfoMac> VideoSourceInfoMac::QueryAllVideoSources()
{
    std::vector<VideoSourceInfoMac> result;

    if (0 != CanAccessVideo()) {
        std::cerr << "Cannot open video device due to privilage" << std::endl;
        return result;
    }

    NSArray* cameras = GetAllCameras();
    for (AVCaptureDevice* device in cameras) {
        VideoSourceInfoMac src;

        ss::String name([device.localizedName UTF8String]);
        ss::String path([device.uniqueID UTF8String]);

        src.cardName = name;
        src.path = path;

        std::vector<CameraConf>& cameraConfs = src.supportedCameraConfs;
        for (AVCaptureDeviceFormat* fmt in device.formats) {
            CameraConf conf;

            auto dimension = CMVideoFormatDescriptionGetDimensions(fmt.formatDescription);
            auto pixelFormat = CMFormatDescriptionGetMediaSubType(fmt.formatDescription);

            auto photonPixelFmt = AVFoundationPixelFormatToPhotonPixelFormat(pixelFormat);
            if (photonPixelFmt == PixelFormat::FMT_UNKNOWN) {
                continue;
            }

            conf.frameFormat.pixelFormat = photonPixelFmt;
            conf.frameFormat.resolution = Resolution { uint32_t(dimension.width), uint32_t(dimension.height) };
            if (nullptr == GetCaptureSessionPresetByResolution(conf.frameFormat.resolution)) {
                continue;
            }

            for (auto* range in [fmt videoSupportedFrameRateRanges]) {
                uint32_t maxFrameRate = uint32_t([range maxFrameRate]);
                uint32_t minFrameRate = uint32_t([range minFrameRate]);
                static const uint32_t kFixFrameRates[] = { 10, 15, 20, 25, 30, 60, 90 };

                bool isMaxFrameRateInlucded = false;
                conf.frameInterval.numerator = 1;
                for (auto i : kFixFrameRates) {
                    if (i >= minFrameRate && i <= maxFrameRate) {
                        conf.frameInterval.denominator = i;
                        cameraConfs.push_back(conf);

                        if (i == maxFrameRate) {
                            isMaxFrameRateInlucded = true;
                        }
                    }
                }

                if (!isMaxFrameRateInlucded) { // ensure the maxFrameRate is included
                    conf.frameInterval.denominator = maxFrameRate;
                    cameraConfs.push_back(conf);
                }
            }
        }
        if (!cameraConfs.empty()) {
            result.push_back(std::move(src));
        }
    }

    return result;
}
