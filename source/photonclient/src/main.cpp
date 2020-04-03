//
// Created by carl on 20-3-31.
//

#include "MediaPlayer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_heler.h"
#include "unistd.h"
#include "video/VideoSource.h"
#include "video/VideoSourceInfo.h"
#include <SSBase/Defer.h>
#include <fcntl.h>
#include <iostream>
#include <libyuv.h>
#include <linux/v4l2-common.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

class MyMediaPlayer : public MediaPlayer {
public:
    explicit MyMediaPlayer(const Config* config)
        : MediaPlayer(config)
    {
        videoSourceInfos_ = VideoSourceInfo::QueryAllVideoSources();
        videoSourceNames_.reserve(videoSourceInfos_.size() + 1);
        videoSourceNames_.emplace_back("[none]");
        for (auto& info : videoSourceInfos_) {
            videoSourceNames_.push_back(info.cardName);
        }
    }

    void UpdateUI() override
    {
        if (ImGui::Begin("MainWindow")) {
            if (ImGui::Combo("Device", &currentSelectedSourceIndex_, videoSourceNames_)) {
                videoSourceFormats_.clear();
                currentSelectedFormatIndex_ = 0;
                videoSource_ = nullptr; // release the former one
                SetVideoFrame(nullptr);

                if (currentSelectedSourceIndex_ != 0) {
                    auto& srcInfo = videoSourceInfos_[currentSelectedSourceIndex_ - 1];
                    videoSourceFormats_.reserve(srcInfo.supportedFrameFormats.size() + 1);
                    videoSourceFormats_.emplace_back("[none]");
                    for (auto& fmt : srcInfo.supportedFrameFormats) {
                        std::string str;
                        str += GetPixelFormatName(fmt.pixelFormat);
                        str += "; ";
                        str += std::to_string(fmt.resolution.width) + "x" + std::to_string(fmt.resolution.height);
                        videoSourceFormats_.push_back(std::move(str));
                    }
                }
            }
            if (currentSelectedSourceIndex_ != 0) {
                // Show formats
                ImGui::Indent();
                OnScopeExit { ImGui::Unindent(); };
                if (ImGui::Combo("Format", &currentSelectedFormatIndex_, videoSourceFormats_)) {
                    videoSource_ = nullptr; // release the former one
                    SetVideoFrame(nullptr);
                    if (currentSelectedFormatIndex_ != 0) {
                        // open stream
                        auto& sourceInfo = videoSourceInfos_[currentSelectedSourceIndex_ - 1];
                        auto& frameFmt = sourceInfo.supportedFrameFormats[currentSelectedFormatIndex_ - 1];

                        videoSource_ = std::make_shared<VideoSource>(sourceInfo.path, frameFmt);
                        videoSource_->SetCallbackPixelFormat(PixelFormat::FMT_YUV420P);
                        videoSource_->SetOnFrame([this](const std::shared_ptr<IVideoFrame>& frame) {
                            SetVideoFrame(frame);
                        });
                        videoSource_->StartCapture();
                    }
                }
            }
        }
        ImGui::End();
    }

private:
    std::vector<VideoSourceInfo> videoSourceInfos_;
    std::vector<std::string> videoSourceNames_;
    int currentSelectedSourceIndex_ { 0 };
    std::vector<std::string> videoSourceFormats_;
    int currentSelectedFormatIndex_ { 0 };

    std::shared_ptr<VideoSource> videoSource_;
};

int main()
{
    MyMediaPlayer::Config config {
        "Photon Client",
        100, 100, 640, 480
    };
    MyMediaPlayer player(&config);
    player.Run();

    std::cout << "client" << std::endl;
    return 0;
}