//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "VideoSourceInfoMac.h"
#include "AVFoundationHelper.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <SSBase/Str.h>
#include <future>

static int CanAccessVideo()
{
    // Request permission to access the camera and microphone.
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo]) {
    case AVAuthorizationStatusAuthorized:
        // The user has previously granted access to the camera.
        return 0;

    case AVAuthorizationStatusNotDetermined: {
        // The app hasn't yet asked the user for camera access.
        std::promise<bool>* p = new std::promise<bool>;
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                                 completionHandler:^(BOOL granted) {
                                     if (granted) {
                                         p->set_value(true);
                                     } else {
                                         p->set_value(false);
                                     }
                                 }];
        bool granted = p->get_future().get();
        delete p;
        if (granted) {
            return 0;
        }
        return 1;
    }
    case AVAuthorizationStatusDenied:
        // The user has previously denied access.
        return 1;
    case AVAuthorizationStatusRestricted:
        // The user can't grant access due to restrictions.
        return 2;
    }
    return 3;
}

//-(AVCaptureDevice *)videoDeviceWitchPosition:(AVCaptureDevicePosition)position
//{
//    AVCaptureDevice *videoDevice;
//
//    if (@available(iOS 11.1, *)) {
//        NSArray<AVCaptureDeviceType> *deviceTypes = @[AVCaptureDeviceTypeBuiltInWideAngleCamera,
//                                                      AVCaptureDeviceTypeBuiltInDualCamera,
//                                                      AVCaptureDeviceTypeBuiltInTrueDepthCamera];
//        AVCaptureDeviceDiscoverySession *session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:deviceTypes
//                                                                                                          mediaType:AVMediaTypeVideo
//                                                                                                           position:position];
//        for (AVCaptureDevice *device in session.devices) {
//            if (device.position == position) {
//                videoDevice = device;
//                break;
//            }
//        }
//    } else if (@available(iOS 10.0, *)) {
//        videoDevice = [AVCaptureDevice defaultDeviceWithDeviceType:AVCaptureDeviceTypeBuiltInWideAngleCamera
//                                                         mediaType:AVMediaTypeVideo
//                                                          position:position];
//    } else {
//        NSArray *cameras = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
//        for (AVCaptureDevice *device in cameras) {
//            if (device.position == position) {
//                videoDevice = device;
//                break;
//            }
//        }
//    }
//
//    return videoDevice;
//}

NSArray* GetAllCameras()
{
    //    if (@available(iOS 11.1, *)) {
    //        NSArray<AVCaptureDeviceType> *deviceTypes = @[AVCaptureDeviceTypeBuiltInWideAngleCamera,
    //                                                      AVCaptureDeviceTypeBuiltInDualCamera,
    //                                                      AVCaptureDeviceTypeBuiltInTrueDepthCamera];
    //        AVCaptureDeviceDiscoverySession *session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:deviceTypes
    //                                                                                                          mediaType:AVMediaTypeVideo
    //                                                                                                           position:position];
    //        for (AVCaptureDevice *device in session.devices) {
    //            if (device.position == position) {
    //                videoDevice = device;
    //                break;
    //            }
    //        }
    //    } else if (@available(iOS 10.0, *)) {
    //        videoDevice = [AVCaptureDevice defaultDeviceWithDeviceType:AVCaptureDeviceTypeBuiltInWideAngleCamera
    //                                                         mediaType:AVMediaTypeVideo
    //                                                          position:position];
    //    } else {
    NSArray* cameras = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    //    }
    return cameras;
}

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

            auto* range = [fmt videoSupportedFrameRateRanges][0];

            auto photonPixelFmt = AVFoundationPixelFormatToPhotonPixelFormat(pixelFormat);
            if (photonPixelFmt == PixelFormat::FMT_UNKNOWN) {
                continue;
            }

            conf.frameFormat.pixelFormat = photonPixelFmt;
            conf.frameFormat.resolution = Resolution { uint32_t(dimension.width), uint32_t(dimension.height) };

            for (auto* range in [fmt videoSupportedFrameRateRanges]) {
                uint32_t maxFrameRate = uint32_t([range maxFrameRate]);
                uint32_t minFrameRate = uint32_t([range minFrameRate]);
                static const uint32_t kFixFrameRates[] = { 1, 5, 10, 15, 20, 25, 30, 60, 90 };

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
