#pragma once

#include "Types.h"

namespace pht {

class Variant {
private:
    enum class Type : uint8_t {
        ByteArray = 1,
        String = 2,
        Array = 3,
        Map = 4,
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

public:
    Variant()
        : type_(Type::Null)
        , asNull_(nullptr)
    {
    }

    // NOTE: This constructor will take the ownship of v
    Variant(ByteArray* v)
        : type_(Type::ByteArray)
        , asByteArray_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(String* v)
        : type_(Type::String)
        , asString_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Array* v)
        : type_(Type::Array)
        , asArray_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(KVArray* v)
        : type_(Type::KVArray)
        , asKVArray_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Uint8* v)
        : type_(Type::Uint8)
        , asUint8_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Int8* v)
        : type_(Type::Int8)
        , asInt8_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Uint8* v)
        : type_(Type::Uint8)
        , asUint8_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Int16* v)
        : type_(Type::Int16)
        , asInt16_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Uint16* v)
        : type_(Type::Uint16)
        , asUint16_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Int32* v)
        : type_(Type::Int32)
        , asInt32_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Uint32* v)
        : type_(Type::Uint32)
        , asUint32_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Int64* v)
        : type_(Type::Int64)
        , asInt64_(v)
    {
    }
    // NOTE: This constructor will take the ownship of v
    Variant(Uint64* v)
        : type_(Type::Uint64)
        , asUint64_(v)
    {
    }

    // NOTE: This constructor will copy the value of v
    Variant(const ByteArray& v);
    // NOTE: This constructor will copy the value of v
    Variant(const String& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Array& v);
    // NOTE: This constructor will copy the value of v
    Variant(const KVArray& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Uint8& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Int8& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Uint8& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Int16& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Uint16& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Int32& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Uint32& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Int64& v);
    // NOTE: This constructor will copy the value of v
    Variant(const Uint64& v);

    ~Variant();

    template <class T>
    bool Is() const;

    // Get the value as type T
    // This function returns the exact value of type T, abort() if type mismatch
    template <class T>
    T& Get() const;

    // Get the value as type T
    // This function returns the exact value of type T.
    // Try to convert the value to cooresponding type if type mismatch.
    // Abort if converting failed.
    template <class T>
    T& As() const;

    // Release the ownship of the data it stores, and return the data pointer.
    // Abort if type mismatch.
    template <class T>
    T* Release();

private:
    Type valueType_;
    union {
        ByteArray* asByteArray_;
        String* asString_;
        Array* asArray_;
        KVArray* asKVArray_;
        Int64* asInt64_;
        Uint64* asUint64_;
        Null* asNull_;
    };
};

} // namespace pht
