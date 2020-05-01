//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/DataSerializer.h"
#include "photonbase/core/Variant.h"
#include "photonbase/protocol/RemoteMethod.h"

namespace pht {

bool DataSerializer::Serialize(const Variant& v, const WriteCallback& write)
{
    write((uint8_t)v.GetType());

    switch (v.GetType()) {
    case Variant::Type::ByteArray: {
        const auto& ba = v.Get<ByteArray>();
        if (!SerializeToDUI<4>((Uint32)ba.Size(), write)) {
            std::cout << "Serialize DUI failed" << std::endl;
            return false;
        }
        for (uint32_t i = 0; i < ba.Size(); ++i) {
            write(ba[i]);
        }
        return true;
    }
    case Variant::Type::String: {
        const auto& str = v.Get<String>();
        return Serialize(str, write);
    }
    case Variant::Type::Array: {
        const auto& arr = v.Get<Array>();
        return Serialize(arr, write);
    }
    case Variant::Type::KVArray: {
        const auto& kvArr = v.Get<KVArray>();
        if (!SerializeToDUI<4>(kvArr.Size(), write)) {
            std::cout << "Serialize DUI failed" << std::endl;
            return false;
        }
        for (uint32_t i = 0; i < kvArr.Size(); ++i) {
            const auto& entry = kvArr[i];
            if (!Serialize(entry.key, write)) {
                return false;
            }
            if (entry.value == nullptr) {
                // Treat nullptr as Null variant
                write(uint8_t(Variant::Type::Null));
                continue;
            }
            if (!Serialize(*entry.value, write)) {
                return false;
            }
        }
        return true;
    }
    case Variant::Type::Int8: {
        const auto byte = v.Get<Int8>();
        write(Uint8(byte));
        return true;
    }
    case Variant::Type::Uint8: {
        const auto byte = v.Get<Uint8>();
        write(byte);
        return true;
    }
    case Variant::Type::Int16: {
        const auto value = (Uint16)v.Get<Int16>();
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Uint16: {
        const auto value = v.Get<Uint16>();
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Int32: {
        const auto value = (Uint32)v.Get<Int32>();
        write(Uint8(value >> 24u));
        write(Uint8(value >> 16u));
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Uint32: {
        const auto value = v.Get<Uint32>();
        write(Uint8(value >> 24u));
        write(Uint8(value >> 16u));
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Int64: {
        const auto value = (Uint64)v.Get<Int64>();
        write(Uint8(value >> 56u));
        write(Uint8(value >> 48u));
        write(Uint8(value >> 40u));
        write(Uint8(value >> 32u));
        write(Uint8(value >> 24u));
        write(Uint8(value >> 16u));
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Uint64: {
        const auto value = v.Get<Uint64>();
        write(Uint8(value >> 56u));
        write(Uint8(value >> 48u));
        write(Uint8(value >> 40u));
        write(Uint8(value >> 32u));
        write(Uint8(value >> 24u));
        write(Uint8(value >> 16u));
        write(Uint8(value >> 8u));
        write(Uint8(value));
        return true;
    }
    case Variant::Type::Null:
        return true;
    default:
        SSASSERT2(false, "Impossible");
    }
}

bool DataSerializer::Serialize(const RemoteMethod& m, const WriteCallback& write)
{
    SSASSERT((m.GetReturnType() >= Variant::Type::ByteArray && m.GetReturnType() <= Variant::Type::Null)
        || m.GetReturnType() == Variant::Type::Void);

    write((uint8_t)m.GetReturnType());
    return Serialize(m.GetMethodName(), write) && Serialize(m.GetParameters(), write);
}

bool DataSerializer::Serialize(const String& str, const WriteCallback& write)
{
    std::string bytes = str.ToStdString(ss::String::CharSet::kUtf8);
    if (!SerializeToDUI<4>(bytes.length(), write)) {
        std::cout << "Serialize DUI failed" << std::endl;
        return false;
    }
    for (auto byte : bytes) {
        write(Uint8(byte));
    }
    return true;
}

bool DataSerializer::Serialize(const Array& arr, const WriteCallback& write)
{
    if (!DataSerializer::SerializeToDUI<4>(arr.Size(), write)) {
        std::cout << "Serialize DUI failed" << std::endl;
        return false;
    }
    for (uint32_t i = 0; i < arr.Size(); ++i) {
        if (arr[i] == nullptr) {
            // Treat nullptr as Null variant
            write(uint8_t(Variant::Type::Null));
            continue;
        }
        if (!DataSerializer::Serialize(*arr[i], write)) {
            return false;
        }
    }
    return true;
}

}
