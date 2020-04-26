//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"
#include <functional>

namespace pht {

class DataDeserializer {
    // clang-format off
    template <int N> struct DUIRange {};
    template <> struct DUIRange<1> { static const Uint8  Max = 255;       static const Uint8  Min = 0; };
    template <> struct DUIRange<2> { static const Uint16 Max = 32767;     static const Uint16 Min = 0; };
    template <> struct DUIRange<3> { static const Uint32 Max = 4194303;   static const Uint32 Min = 0; };
    template <> struct DUIRange<4> { static const Uint32 Max = 536870911; static const Uint32 Min = 0; };
    // clang-format on
public:
    DataDeserializer() = delete;
    ~DataDeserializer() = delete;

    using ReadCallback = std::function<void(const Uint8**, uint32_t)>;

    /**
     *
     * @param v The variant to serialize.
     * @param read A callback function to get binary data.
     * @return Return true on succeed, else false
     */
    static bool Deserialize(Variant& v, const ReadCallback& read);

    /**
     * Serialize an unsigned integer to DUI[N] encoding
     * @tparam N N should be of {1,2,3,4}
     * @tparam T The data type. Should be one of {Uint8, Uint16, Uint32, Uint64}
     * @tparam Max The maximum value DUI[N] can represent(You should not fill this argument). Used to limit template specialization
     * @tparam X Used to limit template specialization
     * @param data The number to serialize
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