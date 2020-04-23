//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "photonbase/Variant.h"
#include <SSBase/Assert.h>

namespace pht {

Int64 Variant::GetIntegerValue() const
{
    switch (type_) {
    case Type::Int8:
        return *reinterpret_cast<Int8*>(data_) & Int64(0xFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Uint8:
        return *reinterpret_cast<Uint8*>(data_) & Int64(0xFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Int16:
        return *reinterpret_cast<Int16*>(data_) & Int64(0xFFFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Uint16:
        return *reinterpret_cast<Uint16*>(data_) & Int64(0xFFFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Int32:
        return *reinterpret_cast<Int32*>(data_) & Int64(0xFFFFFFFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Uint32:
        return *reinterpret_cast<Uint32*>(data_) & Int64(0xFFFFFFFF); // NOLINT(hicpp-signed-bitwise)
    case Type::Int64:
        return *reinterpret_cast<Int64*>(data_);
    case Type::Uint64:
        return (Int64) * reinterpret_cast<Uint64*>(data_);
    default:
        SSASSERT2(false, "The variant is not an integer value");
    }
}

void Variant::SetType(Type newType)
{
    if (newType == type_) {
        return;
    }

    Clear();

    SSASSERT(data_ == nullptr);
    SSASSERT(type_ == Type::Null);
    type_ = newType;

    switch (newType) {
    case Type::ByteArray: {
        data_ = new ByteArray;
        break;
    }
    case Type::String: {
        data_ = new String;
        break;
    }
    case Type::Array: {
        data_ = new Array;
        break;
    }
    case Type::KVArray: {
        data_ = new KVArray;
        break;
    }
    case Type::Int8:
    case Type::Int16:
    case Type::Int32:
    case Type::Int64: {
        data_ = new Int64;
        break;
    }
    case Type::Uint8:
    case Type::Uint16:
    case Type::Uint32:
    case Type::Uint64: {
        data_ = new Uint64;
        break;
    }
    case Type::Null:
        break;
    default:
        std::cerr << "Unexpected variant new type: " << uint8_t(type_) << std::endl;
    }
}

void Variant::Clear()
{
    switch (type_) {
    case Type::ByteArray:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<ByteArray*>(data_);
        break;
    case Type::String:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<String*>(data_);
        break;
    case Type::Array:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<Array*>(data_);
        break;
    case Type::KVArray:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<KVArray*>(data_);
        break;
    case Type::Int8:
    case Type::Int16:
    case Type::Int32:
    case Type::Int64:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<Int64*>(data_);
        break;
    case Type::Uint8:
    case Type::Uint16:
    case Type::Uint32:
    case Type::Uint64:
        SSASSERT(data_ != nullptr);
        delete reinterpret_cast<Uint64*>(data_);
        break;
    case Type::Null:
        break;
    default:
        std::cerr << "Unexpected variant type: " << uint8_t(type_) << std::endl;
    }
    type_ = Type::Null;
    data_ = nullptr;
}

Variant& Variant::operator=(const Variant& v)
{
    if (&v == this) {
        return *this;
    }

    SetType(v.type_);

    switch (type_) {
    case Type::ByteArray:
        Get<ByteArray>() = v.Get<ByteArray>();
        break;
    case Type::String:
        Get<String>() = v.Get<String>();
        break;
    case Type::Array:
        Get<Array>() = v.Get<Array>();
        break;
    case Type::KVArray:
        Get<KVArray>() = v.Get<KVArray>();
        break;
    case Type::Int8:
    case Type::Int16:
    case Type::Int32:
    case Type::Int64:
        *reinterpret_cast<Int64*>(data_) = *reinterpret_cast<Int64*>(v.data_);
        break;
    case Type::Uint8:
    case Type::Uint16:
    case Type::Uint32:
    case Type::Uint64:
        *reinterpret_cast<Uint64*>(data_) = *reinterpret_cast<Uint64*>(v.data_);
        break;
    case Type::Null:
        break;
    default:
        std::cerr << "Unexpected variant type: " << uint8_t(type_) << std::endl;
    }

    return *this;
}

bool operator==(const Variant& a, const Variant& b)
{
    if (&a == &b) {
        return true;
    }
    if (a.GetType() != b.GetType()) {
        return false;
    }
    switch (a.GetType()) {
    case Variant::Type::ByteArray:
        return a.Get<ByteArray>() == b.Get<ByteArray>();
    case Variant::Type::String:
        return a.Get<String>() == b.Get<String>();
    case Variant::Type::Array:
        return a.Get<Array>() == b.Get<Array>();
    case Variant::Type::KVArray:
        return a.Get<KVArray>() == b.Get<KVArray>();
    case Variant::Type::Int8:
        return a.Get<Int8>() == b.Get<Int8>();
    case Variant::Type::Int16:
        return a.Get<Int16>() == b.Get<Int16>();
    case Variant::Type::Int32:
        return a.Get<Int32>() == b.Get<Int32>();
    case Variant::Type::Int64:
        return a.Get<Int64>() == b.Get<Int64>();
    case Variant::Type::Uint8:
        return a.Get<Uint8>() == b.Get<Uint8>();
    case Variant::Type::Uint16:
        return a.Get<Uint16>() == b.Get<Uint16>();
    case Variant::Type::Uint32:
        return a.Get<Uint32>() == b.Get<Uint32>();
    case Variant::Type::Uint64:
        return a.Get<Uint64>() == b.Get<Uint64>();
    case Variant::Type::Null:
        return true;
    default:
        SSASSERT2(false, "Impossible case");
    }
}

} // namespace pht