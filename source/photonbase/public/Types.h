#pragma once

#include <SSBase/Buffer.h>
#include <SSBase/Str.h>
#include <cstdint>
#include <memory>

namespace pht {

template <class T>
class ArrayBase {
public:
    ArrayBase(uint32_t size)
        : size_(size)
        , data_(size > 0 ? new std::shared_ptr<Variant>[size] : nullptr)
    {
    }
    ~ArrayBase()
    {
        if (data_ != nullptr) {
            delete[] data_;
            data_ = nullptr;
        }
    }

    ArrayBase(const ArrayBase&) = delete;
    ArrayBase(ArrayBase&&) = delete;
    ArrayBase& operator=(const ArrayBase&) = delete;
    ArrayBase& operator=(ArrayBase&&) = delete;

    uint32_t Size() const
    {
        return size_;
    }

    std::shared_ptr<Variant>& operator[](uint32_t index)
    {
        SSASSERT(index < size_);
        return data_[index];
    }
    std::shared_ptr<Variant> operator[](uint32_t index) const
    {
        SSASSERT(index < size_);
        return data_[index];
    }

private:
    uint32_t size_;
    T* data_;
};

class Variant;
using ByteArray = ss::DynamicBuffer;
using String = ss::String;
using Array = ArrayBase<std::shared_ptr<Variant>>;
using KVArray = ArrayBase<std::pair<String, std::shared_ptr<Variant>>>;
using Uint8 = std::uint8_t;
using Int8 = std::int8_t;
using Uint8 = std::uint8_t;
using Int16 = std::int16_t;
using Uint16 = std::uint16_t;
using Int32 = std::int32_t;
using Uint32 = std::uint32_t;
using Int64 = std::int64_t;
using Uint64 = std::uint64_t;
using Null = std::nullptr_t;

} // namespace pht
