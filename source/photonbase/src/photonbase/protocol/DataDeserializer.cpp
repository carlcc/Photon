//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/DataDeserializer.h"
#include "photonbase/core/Variant.h"
#include "photonbase/protocol/ChunkHeader.h"
#include "photonbase/protocol/MessageHeader.h"
#include "photonbase/protocol/RemoteMethod.h"

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
        String str;
        if (!Deserialize(str, read)) {
            return false;
        }
        v = std::move(str);
        return true;
    }
    case Uint8(Variant::Type::Array): {
        Array arr;
        if (!Deserialize(arr, read)) {
            return false;
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
            String key;
            if (!Deserialize(key, read)) {
                return false;
            }
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

bool DataDeserializer::Deserialize(RemoteMethod& m, const DataDeserializer::ReadCallback& read)
{
    const Uint8* pRetType;
    READ_NEXT_BYTE(pRetType, 1);
    if ((pRetType[0] < Uint8(Variant::Type::ByteArray) || pRetType[0] > Uint8(Variant::Type::Null))
        && pRetType[0] != Uint8(Variant::Type::Void)) {
        std::cout << "Deserialized failed: Unknown return type: " << Uint8(pRetType[0]) << std::endl;
        return false;
    }

    m.returnType_ = Variant::Type(pRetType[0]);
    return Deserialize(m.methodName_, read) && Deserialize(m.parameters_, read);
}

bool DataDeserializer::Deserialize(ChunkHeader& ch, const DataDeserializer::ReadCallback& read)
{
    return DeserializeFromDUI<2>(ch.channelId, read) && DeserializeFromDUI<4>(ch.chunkId, read) && DeserializeFromDUI<3>(ch.chunkSize, read);
}

bool DataDeserializer::Deserialize(MessageHeader& mh, const DataDeserializer::ReadCallback& read)
{
    bool ret = DeserializeFromDUI<2>(mh.messageId, read) && DeserializeFromDUI<4>(mh.timestamp, read);
    if (!ret) {
        return false;
    }
    const Uint8* pRetType;
    READ_NEXT_BYTE(pRetType, 1);
    mh.reserved = pRetType[0] >> 5u;
    mh.messageType = MessageHeader::Type(pRetType[0] & 0x1Fu);
    return DeserializeFromDUI<4>(mh.messageLength, read);
}

bool DataDeserializer::Deserialize(String& str, const ReadCallback& read)
{
    Uint32 length;
    if (!DeserializeFromDUI<4>(length, read)) {
        return false;
    }
    const Uint8* bytes;
    READ_NEXT_BYTE(bytes, length);
    str = std::move(String((const char*)bytes, length));
    return true;
}

bool DataDeserializer::Deserialize(Array& arr, const ReadCallback& read)
{
    Uint32 length;
    if (!DeserializeFromDUI<4>(length, read)) {
        return false;
    }
    Array tmpArr(length);
    for (Uint32 i = 0; i < length; ++i) {
        auto spVariant = std::make_shared<Variant>();
        tmpArr[i] = spVariant;
        if (!Deserialize(*spVariant, read)) {
            return false;
        }
    }
    arr = std::move(tmpArr);
    return true;
}

}
