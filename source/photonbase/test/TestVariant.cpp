//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "TestVariant.h"
#include <SSBase/Assert.h>
#include <photonbase/core/Variant.h>

#define ASSERT_ABORT(expr)                               \
    try {                                                \
        expr;                                            \
        SSASSERT2(false, "Assert abort failed: " #expr); \
    } catch (...) {                                      \
    }
namespace pht {

void TestVariant::test()
{
    Variant v;
    SSASSERT(v.Is<Null>());
    // SSASSERT(v.Get<Null>()); // not allowed. link error
    ASSERT_ABORT(v.Get<Int64>());

    {
        Variant copy(v);

        Array arr(2);
        arr[0] = std::make_shared<Variant>(String("hello"));
        arr[1] = std::make_shared<Variant>(1);

        Variant v(arr);
        SSASSERT(v.Is<Array>());
        Array& vArr = v.Get<Array>();
        SSASSERT(vArr == arr);

        Variant move(std::move(v));
        SSASSERT(move.Is<Array>());
        Array& vArr2 = move.Get<Array>();
        SSASSERT(&vArr2 == &vArr);

        SSASSERT(vArr2[0]->Get<String>() == "hello");
    }

    std::shared_ptr<KVArray> released = nullptr;
    {
        KVArray arr(2);
        arr[0] = { "Hello", std::make_shared<Variant>(String("world")) };
        arr[1] = { "number", std::make_shared<Variant>(1) };

        Variant v(arr);
        SSASSERT(v.Is<KVArray>());
        KVArray& ref = v.Get<KVArray>();
        SSASSERT(ref == arr);

        Variant copy;
        SSASSERT(copy.Is<Null>());

        copy = v;
        SSASSERT(copy.Is<KVArray>());
        SSASSERT(copy.Get<KVArray>() == arr);

        released.reset(copy.Release<KVArray>());
    }
    SSASSERT(released->At(0).key == "Hello");
    SSASSERT(released->At(0).value->Get<String>() == "world");
    SSASSERT(released->At(1).key == "number");
    SSASSERT(released->At(1).value->As<Int32>() == 1);

    {
        Variant u8(Uint8(128));
        SSASSERT(u8.Is<Uint8>());
        SSASSERT(u8.As<Uint8>() == u8.Get<Uint8>());
        SSASSERT(u8.As<Int8>() == -128);
        SSASSERT(u8.As<Uint16>() == 128);
        SSASSERT(u8.As<Int16>() == 128);
        SSASSERT(u8.As<Uint32>() == 128);
        SSASSERT(u8.As<Int32>() == 128);
        SSASSERT(u8.As<Uint64>() == 128);
        SSASSERT(u8.As<Int64>() == 128);

        Variant i8(Int8(-128));
        SSASSERT(i8.Is<Int8>());
        SSASSERT(i8.As<Uint8>() == 128);
        SSASSERT(i8.As<Int8>() == -128);
        SSASSERT(i8.As<Uint16>() == 128);
        SSASSERT(i8.As<Int16>() == 128);
        SSASSERT(i8.As<Uint32>() == 128);
        SSASSERT(i8.As<Int32>() == 128);
        SSASSERT(i8.As<Uint64>() == 128);
        SSASSERT(i8.As<Int64>() == 128);

        Variant i16(Int16(0x8080));
        SSASSERT(i16.Is<Int16>());
        SSASSERT(i16.As<Uint8>() == 128);
        SSASSERT(i16.As<Int8>() == -128);
        SSASSERT(i16.As<Uint16>() == 0x8080);
        SSASSERT(i16.As<Int16>() == -0x7F80);
        SSASSERT(i16.As<Uint32>() == 0x8080);
        SSASSERT(i16.As<Int32>() == 0x8080);
        SSASSERT(i16.As<Uint64>() == 0x8080);
        SSASSERT(i16.As<Int64>() == 0x8080);
    }

    {
        const Uint8* c = nullptr;
        Variant v(c);
        Variant null(nullptr);

        SSASSERT(v == null);
    }
}

}
