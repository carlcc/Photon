//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"
#include <functional>

namespace pht {

class RemoteMethod;
class ChunkHeader;
class MessageHeader;

class DataDeserializer {
    // clang-format off
    template <int N> struct DUIRange {};
    template <> struct DUIRange<1> { static const Uint8  Max = 255;       static const Uint8  Min = 0; };
    template <> struct DUIRange<2> { static const Uint16 Max = 32767;     static const Uint16 Min = 0; };
    template <> struct DUIRange<3> { static const Uint32 Max = 4194303;   static const Uint32 Min = 0; };
    template <> struct DUIRange<4> { static const Uint32 Max = 536870911; static const Uint32 Min = 0; };
    // clang-format on
public:
    DataDeserializer(void* data, Uint32 available)
        : ptr_(reinterpret_cast<Uint8*>(data))
        , available_(available)
        , dataConsumed_(0)
        , isNotEnoughData_(false)
    {
    }
    ~DataDeserializer() = default;

    bool Deserialize(Variant& v)
    {
        return Deserialize(v, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    bool Deserialize(RemoteMethod& m)
    {
        return Deserialize(m, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    bool Deserialize(ChunkHeader& ch)
    {
        return Deserialize(ch, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    bool Deserialize(MessageHeader& mh)
    {
        return Deserialize(mh, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    bool Deserialize(String& str)
    {
        return Deserialize(str, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    bool Deserialize(Array& arr)
    {
        return Deserialize(arr, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    template <int N, class T, Uint32 Max = DUIRange<N>::Max, class X = IsUnsignedInteger<T>>
    bool DeserializeFromDUI(T& data)
    {
        return DeserializeFromDUI(data, [this](const Uint8** ptr, uint32_t len) {
            ReadFunc(ptr, len);
        });
    }

    Uint32 DataConsumed() const
    {
        return dataConsumed_;
    }

    bool IsNotEnoughData() const
    {
        return isNotEnoughData_;
    }

private:
    void ReadFunc(const Uint8** ptr, uint32_t len)
    {
        if (len > available_) {
            *ptr = nullptr;
            isNotEnoughData_ = true;
            return;
        }
        *ptr = ptr_;
        ptr_ += len;
        dataConsumed_ += len;
        available_ -= len;
    }

private:
    Uint8* ptr_ { nullptr };
    Uint32 available_ { 0 };
    Uint32 dataConsumed_ { 0 };
    bool isNotEnoughData_ { false };

public:
    using ReadCallback = std::function<void(const Uint8**, uint32_t)>;

    /**
     *
     * @param v The variant to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(Variant& v, const ReadCallback& read);

    /**
     *
     * @param m The method to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(RemoteMethod& m, const ReadCallback& read);

    /**
     *
     * @param ch The chunk header to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(ChunkHeader& ch, const ReadCallback& read);

    /**
     *
     * @param mh The message header to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(MessageHeader& mh, const ReadCallback& read);

    /**
     *
     * @param str The String to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(String& str, const ReadCallback& read);

    /**
     *
     * @param arr The array to deserialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(Array& arr, const ReadCallback& read);

    /**
     * Serialize an unsigned integer to DUI[N] encoding
     * @tparam N N should be of {1,2,3,4}
     * @tparam T The data type. Should be one of {Uint8, Uint16, Uint32, Uint64}
     * @tparam Max The maximum value DUI[N] can represent(You should not fill this argument). Used to limit template specialization
     * @tparam X Used to limit template specialization
     * @param data The number to deserialize
     * @param read A callback function to receive serialized bytes.
     * @return Return true on succeed, else false
     */
    template <int N, class T, Uint32 Max = DUIRange<N>::Max, class X = IsUnsignedInteger<T>>
    static bool DeserializeFromDUI(T& data, const ReadCallback& read)
    {
        data = 0;
        const Uint8* ptr;
        for (int i = 0; i < N; ++i) {
            read(&ptr, 1);
            if (ptr == nullptr) {
                return false;
            }
            auto& byte = *ptr;
            if (i == N - 1) {
                data |= byte << (7u * i);
                return true;
            } else {
                data |= (byte & 0x7Fu) << (7u * i);
                if ((byte & 0x80u) == 0) {
                    return true;
                }
            }
        }
        SSASSERT2(false, "Impossible");
    }
};

}