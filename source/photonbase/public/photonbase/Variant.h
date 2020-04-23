#pragma once

#include "Types.h"

namespace pht {

class Variant {
private:
    enum class Type : uint8_t {
        ByteArray = 1,
        String = 2,
        Array = 3,
        KVArray = 4,
        Int8 = 5,
        Uint8 = 6,
        Int16 = 7,
        Uint16 = 8,
        Int32 = 9,
        Uint32 = 10,
        Int64 = 11,
        Uint64 = 12,
        Null = 13,
    };
    // clang-format off
    template <class T> struct VariantTypeTrait;
    template <> struct VariantTypeTrait<ByteArray> { static const Type TypeEnum = Type::ByteArray; };
    template <> struct VariantTypeTrait<String> { static const Type TypeEnum = Type::String; };
    template <> struct VariantTypeTrait<Array> { static const Type TypeEnum = Type::Array; };
    template <> struct VariantTypeTrait<KVArray> { static const Type TypeEnum = Type::KVArray; };
    template <> struct VariantTypeTrait<Int8> { static const Type TypeEnum = Type::Int8; };
    template <> struct VariantTypeTrait<Uint8> { static const Type TypeEnum = Type::Uint8; };
    template <> struct VariantTypeTrait<Int16> { static const Type TypeEnum = Type::Int16; };
    template <> struct VariantTypeTrait<Uint16> { static const Type TypeEnum = Type::Uint16; };
    template <> struct VariantTypeTrait<Int32> { static const Type TypeEnum = Type::Int32; };
    template <> struct VariantTypeTrait<Uint32> { static const Type TypeEnum = Type::Uint32; };
    template <> struct VariantTypeTrait<Int64> { static const Type TypeEnum = Type::Int64; };
    template <> struct VariantTypeTrait<Uint64> { static const Type TypeEnum = Type::Uint64; };
    template <> struct VariantTypeTrait<Null> { static const Type TypeEnum = Type::Null; };
    // clang-format on

public:
    Variant()
        : type_(Type::Null)
        , data_(nullptr)
    {
    }

    // Note: This constructor will take over the ownship of v
    template <class T>
    explicit Variant(T* v)
        : type_(VariantTypeTrait<T>::TypeEnum)
        , data_(v)
    {
        if (v == nullptr) {
            type_ = Type::Null;
        }
    }

    template <class T>
    explicit Variant(const T* v)
        : type_(Type::Null)
        , data_(nullptr)
    {
        if (v != nullptr) {
            *this = *v;
        }
    }

    Variant(const Variant& v)
        : type_(Type::Null)
        , data_(nullptr)
    {
        *this = v;
    }

    Variant(Variant&& v) noexcept
        : type_(v.type_)
        , data_(v.data_)
    {
        v.type_ = Type::Null;
        v.data_ = nullptr;
    }

    // Do not use template for this template will shadow copy and move constructor
    // template <class T>
    // explicit Variant(T&& v)
    //     : type_(VariantTypeTrait<T>::TypeEnum)
    //     , data_(new T(std::forward<T>(v)))
    // {
    // }

    // clang-format off
    Variant(ByteArray&& v): type_(Type::ByteArray), data_(new ByteArray(std::forward<ByteArray>(v))) { } // NOLINT(google-explicit-constructor)
    Variant(String&&    v): type_(Type::String   ), data_(new String   (std::forward<String   >(v))) { } // NOLINT(google-explicit-constructor)
    Variant(Array&&     v): type_(Type::Array    ), data_(new Array    (std::forward<Array    >(v))) { } // NOLINT(google-explicit-constructor)
    Variant(KVArray&&   v): type_(Type::KVArray  ), data_(new KVArray  (std::forward<KVArray  >(v))) { } // NOLINT(google-explicit-constructor)
    Variant(const ByteArray& v): type_(Type::ByteArray), data_(new ByteArray(v)) { } // NOLINT(google-explicit-constructor)
    Variant(const String&    v): type_(Type::String   ), data_(new String   (v)) { } // NOLINT(google-explicit-constructor)
    Variant(const Array&     v): type_(Type::Array    ), data_(new Array    (v)) { } // NOLINT(google-explicit-constructor)
    Variant(const KVArray&   v): type_(Type::KVArray  ), data_(new KVArray  (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Int8        v): type_(Type::Int8     ), data_(new Int8     (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Uint8       v): type_(Type::Uint8    ), data_(new Uint8    (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Int16       v): type_(Type::Int16    ), data_(new Int16    (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Uint16      v): type_(Type::Uint16   ), data_(new Uint16   (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Int32       v): type_(Type::Int32    ), data_(new Int32    (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Uint32      v): type_(Type::Uint32   ), data_(new Uint32   (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Int64       v): type_(Type::Int64    ), data_(new Int64    (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Uint64      v): type_(Type::Uint64   ), data_(new Uint64   (v)) { } // NOLINT(google-explicit-constructor)
    Variant(Null        v): type_(Type::Null     ), data_(nullptr         ) { } // NOLINT(google-explicit-constructor)
    // clang-format on

    ~Variant() { Clear(); }
    // clang-format on

    Variant& operator=(const Variant& v);
    Variant& operator=(Variant&& v) noexcept
    {
        std::swap(type_, v.type_);
        std::swap(data_, v.data_);
        return *this;
    }

    // Do not use this template for it may shadow copy and move assign
    // template <class T>
    // Variant& operator=(T&& v)
    // {
    //     SetType(VariantTypeTrait<T>::TypeEnum);
    //     *reinterpret_cast<T*>(data_) = std::forward<T>(v);
    //     return *this;
    // }

    // clang-format on
    Variant& operator=(ByteArray&& v)
    {
        SetType(Type::ByteArray);
        *reinterpret_cast<ByteArray*>(data_) = std::forward<ByteArray>(v);
        return *this;
    }
    Variant& operator=(String&& v)
    {
        SetType(Type::String);
        *reinterpret_cast<String*>(data_) = std::forward<String>(v);
        return *this;
    }
    Variant& operator=(Array&& v)
    {
        SetType(Type::Array);
        *reinterpret_cast<Array*>(data_) = std::forward<Array>(v);
        return *this;
    }
    Variant& operator=(KVArray&& v)
    {
        SetType(Type::KVArray);
        *reinterpret_cast<KVArray*>(data_) = std::forward<KVArray>(v);
        return *this;
    }
    Variant& operator=(Int8 v)
    {
        SetType(Type::Int8);
        *reinterpret_cast<Int8*>(data_) = std::forward<Int8>(v);
        return *this;
    }
    Variant& operator=(Uint8 v)
    {
        SetType(Type::Uint8);
        *reinterpret_cast<Uint8*>(data_) = std::forward<Uint8>(v);
        return *this;
    }
    Variant& operator=(Int16 v)
    {
        SetType(Type::Int16);
        *reinterpret_cast<Int16*>(data_) = std::forward<Int16>(v);
        return *this;
    }
    Variant& operator=(Uint16 v)
    {
        SetType(Type::Uint16);
        *reinterpret_cast<Uint16*>(data_) = std::forward<Uint16>(v);
        return *this;
    }
    Variant& operator=(Int32 v)
    {
        SetType(Type::Int32);
        *reinterpret_cast<Int32*>(data_) = std::forward<Int32>(v);
        return *this;
    }
    Variant& operator=(Uint32 v)
    {
        SetType(Type::Uint32);
        *reinterpret_cast<Uint32*>(data_) = std::forward<Uint32>(v);
        return *this;
    }
    Variant& operator=(Int64 v)
    {
        SetType(Type::Int64);
        *reinterpret_cast<Int64*>(data_) = std::forward<Int64>(v);
        return *this;
    }
    Variant& operator=(Uint64 v)
    {
        SetType(Type::Uint64);
        *reinterpret_cast<Uint64*>(data_) = std::forward<Uint64>(v);
        return *this;
    }
    Variant& operator=(Null v)
    {
        SetType(Type::Null);
        return *this;
    }

    Type GetType() const
    {
        return type_;
    }

    template <class T>
    bool Is() const
    {
        return VariantTypeTrait<T>::TypeEnum == type_;
    }

    bool IsInteger() const
    {
        return Type::Int8 == type_ || Type::Uint8 == type_ || Type::Int16 == type_ || Type::Uint16 == type_ || Type::Int32 == type_ || Type::Uint32 == type_ || Type::Int64 == type_;
    }

    // Get the value as type T
    // This function returns the exact value of type T, abort() if type mismatch
    template <class T>
    const T& Get() const
    {
        SSASSERT(Is<T>());
        return *reinterpret_cast<T*>(data_);
    }

    template <class T>
    T& Get()
    {
        SSASSERT(Is<T>());
        return *reinterpret_cast<T*>(data_);
    }

    // Get<Null> is not allowd
    template <>
    const Null& Get<Null>() const;
    template <>
    Null& Get<Null>();

    // Get the value as type T
    // This function returns the exact value of type T.
    // Try to convert the value to corresponding type if type mismatch.
    // Abort if converting failed.
    // NOTE: Only integer types can used
    template <class T>
    T As() const;

    // clang-format off
    template <> Int8   As<Int8  >() const { return (Int8  ) GetIntegerValue(); }
    template <> Uint8  As<Uint8 >() const { return (Uint8 ) GetIntegerValue(); }
    template <> Int16  As<Int16 >() const { return (Int16 ) GetIntegerValue(); }
    template <> Uint16 As<Uint16>() const { return (Uint16) GetIntegerValue(); }
    template <> Int32  As<Int32 >() const { return (Int32 ) GetIntegerValue(); }
    template <> Uint32 As<Uint32>() const { return (Uint32) GetIntegerValue(); }
    template <> Int64  As<Int64 >() const { return (Int64 ) GetIntegerValue(); }
    template <> Uint64 As<Uint64>() const { return (Uint64) GetIntegerValue(); }
    // clang-format on

    // Release the ownship of the data it stores, and return the data pointer.
    // Abort if type mismatch.
    template <class T>
    T* Release()
    {
        SSASSERT(Is<T>());
        auto* ptr = reinterpret_cast<T*>(data_);
        type_ = Type::Null;
        data_ = nullptr;
        return ptr;
    }

    template <>
    Null* Release()
    {
        SSASSERT(Is<Null>());
        return nullptr;
    }

private:
    Int64 GetIntegerValue() const;

    void SetType(Type newType);

    void Clear();

private:
    Type type_;
    void* data_;

    friend bool operator==(const Variant& a, const Variant& b);
};

bool operator==(const Variant& a, const Variant& b);

} // namespace pht
