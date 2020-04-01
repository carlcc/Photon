//
// Created by carl on 20-4-1.
//
#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <memory>

class IVideoFrame {
public:
    virtual const uint8_t* Y() const = 0;
    virtual const uint8_t* U() const = 0;
    virtual const uint8_t* V() const = 0;

    virtual uint32_t YStride() const = 0;
    virtual uint32_t UStride() const = 0;
    virtual uint32_t VStride() const = 0;

    virtual uint32_t Width() const = 0;
    virtual uint32_t Height() const = 0;
};

class MediaPlayer {
public:
    struct Config {
        const char* title { "" };
        int x { 0 };
        int y { 0 };
        int width { 640 };
        int height { 480 };
    };

    explicit MediaPlayer(Config* config);

    ~MediaPlayer();

    bool Start();

    void Stop(bool waitUntilThreadStop = true);

    void SetVideoFrame(const std::shared_ptr<IVideoFrame>& frame);

private:
    class Impl;
    Impl* impl_;
};
