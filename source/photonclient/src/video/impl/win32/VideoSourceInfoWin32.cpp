//
// Created by carl on 2020/4/4.
//

#include "VideoSourceInfoWin32.h"
#include "DShowHelper.h"
#include <SSBase/Str.h>
#include <Windows.h>
#include <algorithm>
#include <dshowcapture.hpp>
#include <iostream>
#include <strmif.h>

// https://docs.microsoft.com/en-us/windows/win32/directshow/selecting-a-capture-device

class Photon_CoInitializeHelper {
public:
    Photon_CoInitializeHelper()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (hr != S_OK) {
            std::cerr << "CoInitialize failed." << std::endl;
        }
    }
    ~Photon_CoInitializeHelper()
    {
        CoUninitialize();
    }

    static const Photon_CoInitializeHelper& Instance()
    {
        static Photon_CoInitializeHelper s;
        return s;
    }
};

inline uint32_t MinPower10GreaterThan(uint32_t n)
{
    uint32_t i = 1;
    while (i < n) {
        i *= 10;
    }
    return i;
}

std::vector<VideoSourceInfoWin32> VideoSourceInfoWin32::QueryAllVideoSources()
{
    Photon_CoInitializeHelper::Instance();

    std::vector<VideoSourceInfoWin32> result;

    std::vector<DShow::VideoDevice> devices;
    bool suc = DShow::Device::EnumVideoDevices(devices);
    if (!suc) {
        return result;
    }

    for (auto& device : devices) {
        VideoSourceInfoWin32 src {};
        src.cardName = ss::String(device.name.c_str());
        src.path = ss::String(device.path.c_str());

        std::vector<CameraConf>& cameraConfs = src.supportedCameraConfs;
        for (auto& cap : device.caps) {
            CameraConf conf {};
            conf.frameFormat.pixelFormat = DShowPixelFormatToPhotonPixelFormat(cap.format);
            if (conf.frameFormat.pixelFormat == PixelFormat::FMT_UNKNOWN) {
                continue;
            }
            conf.frameFormat.resolution = Resolution { uint32_t(cap.maxCX), uint32_t(cap.maxCY) };

            conf.frameInterval.numerator = cap.minInterval;
            conf.frameInterval.denominator = 10000000;
            cameraConfs.push_back(conf);

            if (cap.minInterval != cap.maxInterval) {
                conf.frameInterval.numerator = cap.minInterval;
                cameraConfs.push_back(conf);
            }
        }
        if (cameraConfs.empty()) {
            continue;
        }

        std::sort(cameraConfs.begin(), cameraConfs.end(), [](const CameraConf& a, const CameraConf& b) {
            return a.frameFormat.resolution < b.frameFormat.resolution;
        });
        auto end = std::unique(cameraConfs.begin(), cameraConfs.end());
        cameraConfs.erase(end, cameraConfs.end());

        result.push_back(std::move(src));
    }

    return result;
}
