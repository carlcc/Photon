//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/DataDeserializer.h"
#include "photonbase/Variant.h"

namespace pht {

#define READ_NEXT_BYTE(bytesPtr, n) \
    do {                            \
        read(&bytesPtr, n);         \
        if (bytesPtr == nullptr) {  \
            return false;           \
        }                           \
    } while (false)

bool pht::DataDeserializer::Deserialize(Variant& v, const ReadCallback& read)
{
    const Uint8* pType;
    READ_NEXT_BYTE(pType, 1);

    Uint8 type = pType[0];
    switch (type) {
    case Uint8(Variant::Type::ByteArray): {
        Uint32 length;
        if (!DeserializeFromDUI<4>(length, read)) {
            return false;
        }
        const Uint8* bytes;
        ByteArray arr(length);
        READ_NEXT_BYTE(bytes, length);
        memcpy(arr.Data(), bytes, length);
        v = std::move(arr);
        return true;
    }
    case Uint8(Variant::Type::String): {
        Uint32 length;
        if (!DeserializeFromDUI<4>(length, read)) {
            return false;
        }
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, length);
        String str((const char*)bytes, length);
        v = std::move(str);
        return true;
    }
    case Uint8(Variant::Type::Array): {
        Uint32 length;
        if (!DeserializeFromDUI<4>(length, read)) {
            return false;
        }
        Array arr(length);
        for (Uint32 i = 0; i < length; ++i) {
            auto spVariant = std::make_shared<Variant>();
            arr[i] = spVariant;
            if (!Deserialize(*spVariant, read)) {
                return false;
            }
        }
        v = std::move(arr);
        return true;
    }
    case Uint8(Variant::Type::KVArray): {
        Uint32 length;
        if (!DeserializeFromDUI<4>(length, read)) {
            return false;
        }
        KVArray arr(length);
        for (Uint32 i = 0; i < length; ++i) {
            Uint32 keyLength;
            if (!DeserializeFromDUI<4>(keyLength, read)) {
                return false;
            }
            const Uint8* bytes;
            READ_NEXT_BYTE(bytes, length);
            String key((const char*)bytes, length);

            auto spVariant = std::make_shared<Variant>();
            if (!Deserialize(*spVariant, read)) {
                return false;
            }

            arr[i] = { std::move(key), spVariant };
        }
        v = std::move(arr);
        return true;
    }
    case Uint8(Variant::Type::Int8): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, 1);
        v = Int8(*bytes);
        return true;
    }
    case Uint8(Variant::Type::Uint8): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Uint8));
        v = Uint8(*bytes);
        return true;
    }
    case Uint8(Variant::Type::Int16): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Int16));
        v = Int16(Uint32(bytes[0]) << 8u | bytes[1]);
        return true;
    }
    case Uint8(Variant::Type::Uint16): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Uint16));
        v = Uint16(Uint32(bytes[0]) << 8u | bytes[1]);
        return true;
    }
    case Uint8(Variant::Type::Int32): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Int32));
        v = Int32(Uint32(bytes[0]) << 24u | Uint32(bytes[1]) << 16u | Uint32(bytes[2]) << 8u | bytes[3]);
        return true;
    }
    case Uint8(Variant::Type::Uint32): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Uint32));
        v = Uint32(Uint32(bytes[0]) << 24u | Uint32(bytes[1]) << 16u | Uint32(bytes[2]) << 8u | bytes[3]);
        return true;
    }
    case Uint8(Variant::Type::Int64): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Int64));
        v = Int64(Uint64(bytes[0]) << 56u | Uint64(bytes[1]) << 48u | Uint64(bytes[2]) << 40u | Uint64(bytes[3]) << 32u
            | Uint64(bytes[4]) << 24u | Uint64(bytes[5]) << 16u | Uint64(bytes[6]) << 8u | bytes[7]);
        return true;
    }
    case Uint8(Variant::Type::Uint64): {
        const Uint8* bytes;
        READ_NEXT_BYTE(bytes, sizeof(Uint64));
        v = Uint64(Uint64(bytes[0]) << 56u | Uint64(bytes[1]) << 48u | Uint64(bytes[2]) << 40u | Uint64(bytes[3]) << 32u
            | Uint64(bytes[4]) << 24u | Uint64(bytes[5]) << 16u | Uint64(bytes[6]) << 8u | bytes[7]);
        return true;
    }
    case Uint8(Variant::Type::Null): {
        return true;
    }
    default:
        std::cerr << "Deserialize failed: invalid variant type: " << int(type) << std::endl;
        return false;
    }
}

}
