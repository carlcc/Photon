//
// Created by carl on 20-3-31.
//
#define SDL_MAIN_HANDLED
#include "MediaPlayer.h"
#include "audio/AudioDestination.h"
#include "audio/AudioDestinationInfo.h"
#include "audio/AudioSource.h"
#include "audio/AudioSourceInfo.h"
#include "imgui/imgui.h"
#include "imgui/imgui_heler.h"
#include "video/VideoSource.h"
#include "video/VideoSourceInfo.h"
#include <SSBase/Defer.h>
#include <SSBase/Time.h>
#include <iostream>

class MyMediaPlayer : public MediaPlayer, public std::enable_shared_from_this<MyMediaPlayer> {
public:
    explicit MyMediaPlayer(const Config* config)
        : MediaPlayer(config)
    {
        videoSourceInfos_ = VideoSourceInfo::QueryAllVideoSources();
        videoSourceNames_.reserve(videoSourceInfos_.size() + 1);
        videoSourceNames_.emplace_back("[none]");
        for (auto& info : videoSourceInfos_) {
            videoSourceNames_.push_back(info.cardName.ToStdString());
        }

        audioSourceInfos_ = AudioSourceInfo::QueryAllAudioSources();
        audioSourceNames_.reserve(audioSourceInfos_.size() + 1);
        audioSourceNames_.emplace_back("[none]");
        for (auto& info : audioSourceInfos_) {
            audioSourceNames_.push_back(info.cardName);
        }

        audioDestinationInfos_ = AudioDestinationInfo::QueryAllAudioDestinations();
        audioDestinationNames_.reserve(audioDestinationInfos_.size() + 1);
        audioDestinationNames_.emplace_back("[none]");
        for (auto& info : audioDestinationInfos_) {
            audioDestinationNames_.push_back(info.cardName);
        }
    }

    void UpdateVideo()
    {
        if (ImGui::Combo("Video Device", &currentSelectedSourceIndex_, videoSourceNames_)) {
            videoSourceFormats_.clear();
            currentSelectedFormatIndex_ = 0;
            videoSource_ = nullptr; // release the former one
            SetVideoFrame(nullptr);

            if (currentSelectedSourceIndex_ != 0) {
                auto& srcInfo = videoSourceInfos_[currentSelectedSourceIndex_ - 1];
                videoSourceFormats_.reserve(srcInfo.supportedCameraConfs.size() + 1);
                videoSourceFormats_.emplace_back("[none]");
                for (auto& conf : srcInfo.supportedCameraConfs) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%s; %ux%u; %.1f fps",
                        GetPixelFormatName(conf.frameFormat.pixelFormat),
                        conf.frameFormat.resolution.width, conf.frameFormat.resolution.height,
                        1.0 * conf.frameInterval.denominator / conf.frameInterval.numerator);
                    videoSourceFormats_.emplace_back(buf);
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
                    auto& cameraConf = sourceInfo.supportedCameraConfs[currentSelectedFormatIndex_ - 1];

                    videoSource_ = std::make_shared<VideoSource>(sourceInfo, cameraConf);
                    videoSource_->SetCallbackPixelFormat(PixelFormat::FMT_YUV420P);
                    videoSource_->SetOnFrame([weakThis = weak_from_this()](const std::shared_ptr<IVideoFrame>& frame) {
                        auto sp = weakThis.lock();
                        if (sp) {
                            sp->SetVideoFrame(frame);
                        }
                    });
                    videoSource_->StartCapture();
                }
            }
        }
    }

    void UpdateAudio()
    {
        if (ImGui::Combo("Audio input Device", &currentAudioSourceIndex_, audioSourceNames_)) {
            audioSourceFormats_.clear();
            currentAudioFormatIndex_ = 0;
            audioSource_ = nullptr; // release the former one

            if (currentAudioSourceIndex_ != 0) {
                auto& srcInfo = audioSourceInfos_[currentAudioSourceIndex_ - 1];
                audioSourceFormats_.reserve(srcInfo.supportedMicConfs.size() + 1);
                audioSourceFormats_.emplace_back("[none]");
                for (auto& conf : srcInfo.supportedMicConfs) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%s; %.1f Hz, %u",
                        srcInfo.cardName.c_str(),
                        conf.sampleRate,
                        conf.channelCount);
                    audioSourceFormats_.emplace_back(buf);
                }
            }
        }
        if (currentAudioSourceIndex_ != 0) {
            // Show formats
            ImGui::Indent();
            OnScopeExit { ImGui::Unindent(); };
            if (ImGui::Combo("Format", &currentAudioFormatIndex_, audioSourceFormats_)) {
                audioSource_ = nullptr; // release the former one
                if (currentAudioFormatIndex_ != 0) {
                    // open stream
                    auto& sourceInfo = audioSourceInfos_[currentAudioSourceIndex_ - 1];
                    auto& micConf = sourceInfo.supportedMicConfs[currentAudioFormatIndex_ - 1];

                    audioSource_ = std::make_shared<AudioSource>(sourceInfo.path, micConf);
                    audioSource_->SetOnAudioFrame([weakThis = weak_from_this()](const int16_t* data, uint32_t frameCount) {
                        auto sp = weakThis.lock();
                        if (sp != nullptr && sp->audioDestination_ != nullptr) {
                           sp->audioDestination_->WriteSounds(data, frameCount);
                        }
                    });
                    audioSource_->StartCapture();
                }
            }
        }
    }

    void UpdateAudioDst()
    {
        if (ImGui::Combo("Audio output Device", &currentAudioDestinationIndex_, audioDestinationNames_)) {
            audioDestination_ = nullptr;
            if (currentAudioDestinationIndex_ != 0) {
                const auto& conf = audioDestinationInfos_[currentAudioDestinationIndex_ - 1];
                auto& sourceInfo = audioSourceInfos_[currentAudioSourceIndex_ - 1];
                auto& micConf = sourceInfo.supportedMicConfs[currentAudioFormatIndex_ - 1];
                audioDestination_ = std::make_shared<AudioDestination>(conf.path, micConf);
                audioDestination_->StartPlay();
            }
        }
    }

    void UpdateUI() override
    {
        if (ImGui::Begin("MainWindow")) {
            ImGui::PushID("Video");
            UpdateVideo();
            ImGui::PopID();

            ImGui::PushID("AudioSrc");
            UpdateAudio();
            ImGui::PopID();

            ImGui::PushID("AudioDst");
            UpdateAudioDst();
            ImGui::PopID();
        }
        ImGui::End();
    }

private:
    // video
    std::vector<VideoSourceInfo> videoSourceInfos_;
    std::vector<std::string> videoSourceNames_;
    int currentSelectedSourceIndex_ { 0 };
    std::vector<std::string> videoSourceFormats_;
    int currentSelectedFormatIndex_ { 0 };

    std::shared_ptr<VideoSource> videoSource_ { nullptr };

    // audio
    std::vector<AudioSourceInfo> audioSourceInfos_;
    std::vector<std::string> audioSourceNames_;
    int currentAudioSourceIndex_ { 0 };
    std::vector<std::string> audioSourceFormats_;
    int currentAudioFormatIndex_ { 0 };

    std::shared_ptr<AudioSource> audioSource_ { nullptr };

    // audio output
    std::vector<AudioDestinationInfo> audioDestinationInfos_;
    std::vector<std::string> audioDestinationNames_;
    int currentAudioDestinationIndex_ { 0 };

    std::shared_ptr<AudioDestination> audioDestination_ { nullptr };
};

int main()
{
    MyMediaPlayer::Config config {
        "Photon Client",
        100, 100, 640, 480
    };
    auto player = std::make_shared<MyMediaPlayer>(&config);
    player->Run();

    std::cout << "client" << std::endl;
    return 0;
}