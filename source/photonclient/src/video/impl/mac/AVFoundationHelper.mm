//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "AVFoundationHelper.h"
#import <AVFoundation/AVFoundation.h>
#include <future>

PixelFormat AVFoundationPixelFormatToPhotonPixelFormat(uint32_t format)
{
    switch (format) {
    case kCVPixelFormatType_422YpCbCr8_yuvs:
        return PixelFormat::FMT_YUYV422;
    default:
        return PixelFormat::FMT_UNKNOWN;
    }
}

uint32_t PhotonPixelFormatToAVFoundationPixelFormat(PixelFormat format)
{
    switch (format) {
    case PixelFormat::FMT_YUYV422:
        return kCVPixelFormatType_422YpCbCr8_yuvs;

    default:
        return -1;
    }
}

int CanAccessVideo()
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
    NSArray* cameras = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    return cameras;
}
