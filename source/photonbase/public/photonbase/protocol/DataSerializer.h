//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"
#include <functional>

namespace pht {

class Variant;

class DataSerializer {
    // clang-format off
    template <int N> struct DUIRange {};
    template <> struct DUIRange<1> { static const Uint8  Max = 255;       static const Uint8  Min = 0; };
    template <> struct DUIRange<2> { static const Uint16 Max = 32767;     static const Uint16 Min = 0; };
    template <> struct DUIRange<3> { static const Uint32 Max = 4194303;   static const Uint32 Min = 0; };
    template <> struct DUIRange<4> { static const Uint32 Max = 536870911; static const Uint32 Min = 0; };
    // clang-format on
public:
    DataSerializer() = delete;
    ~DataSerializer() = delete;

    using WriteCallback = std::function<void(uint8_t)>;

    /**
     *
     * @param v The variant to serialize.
     * @param write A callback function to receive serialized bytes.
     * @return Return true on succeed, else false
     */
    static bool Serialize(const Variant& v, const WriteCallback& write);

    /**
     * Serialize an unsigned integer to DUI[N] encoding
     * @tparam N N should be of {1,2,3,4}
     * @tparam T The data type. Should be one of {Uint8, Uint16, Uint32, Uint64}
     * @tparam Max The maximum value DUI[N] can represent(You should not fill this argument). Used to limit template specialization
     * @tparam X Used to limit template specialization
     * @param data The number to serialize
     * @param write A callback function to receive serialized bytes.
     * @return Return true on succeed, else false
     */
    template <int N, class T, Uint32 Max = DUIRange<N>::Max, class X = IsUnsignedInteger<T>>
    static bool SerializeToDUI(T data, const WriteCallback& write)
    {
        if (data > Max) {
            return false;
        }
        for (int i = 1; i < N; ++i) {
            if (data < 0x80) {
                write(Uint8(data));
                return true;
            }
            write(Uint8(data | 0x80));
            data >>= 7;
        }
        SSASSERT(data < 256);
        write(Uint8(data));
        return true;
    }
};

}