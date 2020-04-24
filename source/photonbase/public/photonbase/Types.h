#pragma once

#include <SSBase/Assert.h>
#include <SSBase/Str.h>
#include <cstdint>
#include <memory>

namespace pht {

template <class T>
class ArrayBase {
public:
    explicit ArrayBase(uint32_t size = 0)
        : size_(size)
        , data_(size > 0 ? new T[size] : nullptr)
    {
    }
    ~ArrayBase()
    {
        if (data_ != nullptr) {
            delete[] data_;
            data_ = nullptr;
        }
    }

    ArrayBase(const ArrayBase& a)
        : size_(a.Size())
        , data_(size_ > 0 ? new T[size_] : nullptr)
    {
        for (uint32_t i = 0; i < size_; ++i) {
            data_[i] = a[i];
        }
    }
    ArrayBase(ArrayBase&& a) noexcept
        : size_(a.Size())
        , data_(a.data_)
    {
        a.size_ = 0;
        a.data_ = nullptr;
    }
    ArrayBase& operator=(const ArrayBase& a)
    {
        if (&a == this) {
            return *this;
        }
        if (size_ != a.size_) {
            delete[] data_;
            size_ = a.size_;
            data_ = size_ > 0 ? new T[size_] : nullptr;
        }
        for (uint32_t i = 0; i < size_; ++i) {
            data_[i] = a[i];
        }
        return *this;
    }
    ArrayBase& operator=(ArrayBase&& a) noexcept
    {
        std::swap(size_, a.size_);
        std::swap(data_, a.data_);
        return *this;
    }

    uint32_t Size() const
    {
        return size_;
    }

    T& At(uint32_t index)
    {
        SSASSERT(index < size_);
        return data_[index];
    }

    T At(uint32_t index) const
    {
        SSASSERT(index < size_);
        return data_[index];
    }

    T& operator[](uint32_t index)
    {
        return At(index);
    }
    T operator[](uint32_t index) const
    {
        return At(index);
    }

private:
    uint32_t size_;
    T* data_;
};

template <class T>
bool operator==(const ArrayBase<T>& a, const ArrayBase<T>& b)
{
    if (&a == &b) {
        return true;
    }
    if (a.Size() != b.Size()) {
        return false;
    }
    for (uint32_t i = 0; i < a.Size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

using ByteArray = ArrayBase<uint8_t>;
using String = ss::String;
class Variant;
struct KVEntry {
    String key;
    std::shared_ptr<Variant> value;
};
inline bool operator==(const KVEntry& a, const KVEntry& b)
{
    return a.key == b.key && a.value == b.value;
}
inline bool operator!=(const KVEntry& a, const KVEntry& b)
{
    return !(a == b);
}
using Array = ArrayBase<std::shared_ptr<Variant>>;
using KVArray = ArrayBase<KVEntry>;
using Int8 = std::int8_t;
using Uint8 = std::uint8_t;
using Int16 = std::int16_t;
using Uint16 = std::uint16_t;
using Int32 = std::int32_t;
using Uint32 = std::uint32_t;
using Int64 = std::int64_t;
using Uint64 = std::uint64_t;
using Null = std::nullptr_t;

// clang-format off
template <class T> struct IsUnsignedInteger {};
template <> struct IsUnsignedInteger<Uint8>  { using Type = Uint8; };
template <> struct IsUnsignedInteger<Uint16> { using Type = Uint16; };
template <> struct IsUnsignedInteger<Uint32> { using Type = Uint32; };
template <> struct IsUnsignedInteger<Uint64> { using Type = Uint64; };

template <class T> struct IsSignedInteger {};
template <> struct IsSignedInteger<Int8>  { using Type = Int8; };
template <> struct IsSignedInteger<Int16> { using Type = Int16; };
template <> struct IsSignedInteger<Int32> { using Type = Int32; };
template <> struct IsSignedInteger<Int64> { using Type = Int64; };

template <class T> struct IsInteger {};
template <> struct IsInteger<Uint8>  { using Type = Uint8; };
template <> struct IsInteger<Uint16> { using Type = Uint16; };
template <> struct IsInteger<Uint32> { using Type = Uint32; };
template <> struct IsInteger<Uint64> { using Type = Uint64; };
template <> struct IsInteger<Int8>   { using Type = Int8; };
template <> struct IsInteger<Int16>  { using Type = Int16; };
template <> struct IsInteger<Int32>  { using Type = Int32; };
template <> struct IsInteger<Int64>  { using Type = Int64; };
// clang-format on
} // namespace pht
