//
// Created by carl on 20-4-2.
//
#pragma once
#include <cstdint>

class IVideoFrame {
public:
    virtual ~IVideoFrame() {};
    virtual uint8_t* Y() = 0;
    virtual uint8_t* U() = 0;
    virtual uint8_t* V() = 0;

    const uint8_t* Y() const
    {
        return ((IVideoFrame*)this)->Y();
    }
    const uint8_t* U() const
    {
        return ((IVideoFrame*)this)->U();
    }
    const uint8_t* V() const
    {
        return ((IVideoFrame*)this)->V();
    }

    virtual uint32_t YStride() const = 0;
    virtual uint32_t UStride() const = 0;
    virtual uint32_t VStride() const = 0;

    virtual uint32_t Width() const = 0;
    virtual uint32_t Height() const = 0;
};

class YUV420PVideoFrame : public IVideoFrame {
public:
    YUV420PVideoFrame(uint32_t w, uint32_t h)
    {
        buffer_ = new uint8_t[w * h + w * h / 2];
        width = w;
        height = h;
    }
    ~YUV420PVideoFrame() override
    {
        delete buffer_;
    }
    uint8_t* Y() override
    {
        return buffer_;
    }
    uint8_t* U() override
    {
        return Y() + YStride() * height;
    }
    uint8_t* V() override
    {
        return U() + UStride() * height/2;
    }
    uint32_t YStride() const override
    {
        return width;
    }
    uint32_t UStride() const override
    {
        return width / 2;
    }
    uint32_t VStride() const override
    {
        return width / 2;
    }
    uint32_t Width() const override
    {
        return width;
    }
    uint32_t Height() const override
    {
        return height;
    }

    //private:
    uint8_t* buffer_;
    uint32_t width;
    uint32_t height;
};