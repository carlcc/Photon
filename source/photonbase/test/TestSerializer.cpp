//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "TestSerializer.h"

#include "photonbase/DataDeserializer.h"
#include "photonbase/DataSerializer.h"
#include "photonbase/Variant.h"

namespace pht {

bool Equals(const Array& a, const Array& b)
{
    if (a.Size() != b.Size()) {
        return false;
    }
    for (Uint32 i = 0; i < a.Size(); ++i) {
        auto spa = a[i];
        auto spb = b[i];
        if (spa == nullptr || spa->Is<Null>()) {
            if (spb == nullptr || spb->Is<Null>()) {
                continue;
                ;
            }
            return false;
        }
        if (*spa != *spb) {
            return false;
        }
    }
    return true;
}

bool Equals(const KVArray& a, const KVArray& b)
{
    if (a.Size() != b.Size()) {
        return false;
    }
    for (Uint32 i = 0; i < a.Size(); ++i) {
        auto& entryA = a[i];
        auto& entryB = b[i];
        if (entryA.key != entryB.key) {
            return false;
        }

        auto spa = entryA.value;
        auto spb = entryB.value;
        if (spa == nullptr || spa->Is<Null>()) {
            if (spb == nullptr || spb->Is<Null>()) {
                continue;
                ;
            }
            return false;
        }
        if (*spa != *spb) {
            return false;
        }
    }
    return true;
}

bool Equals(const Variant& a, const Variant& b)
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
        return Equals(a.Get<Array>(), b.Get<Array>());
    case Variant::Type::KVArray:
        return Equals(a.Get<KVArray>(), b.Get<KVArray>());
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

void DEFINE_VARIANT_TEST_CASE(const Variant& variant, const std::vector<uint8_t>& expected)
{
    {
        uint8_t buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        uint32_t index = 0;
        DataSerializer::Serialize(variant, [&buffer, &index](uint8_t b) {
            buffer[index++] = b;
        });
        SSASSERT(index == expected.size());
        SSASSERT(memcmp(buffer, expected.data(), index) == 0);
    }
    {
        // Test Deserialize
        Variant dV;
        Uint32 index = 0;
        DataDeserializer::Deserialize(dV, [&expected, &index](const Uint8** ptr, Uint32 size) {
            if (index + size > expected.size()) {
                *ptr = nullptr;
            }
            *ptr = expected.data() + index;
            index += size;
        });

        SSASSERT(Equals(dV, variant));
    }
}

template <int N>
void DEFINE_DUI_TEST_CASE(Uint32 n, const std::vector<uint8_t>& expected)
{
    {
        uint8_t buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        uint32_t index = 0;
        DataSerializer::SerializeToDUI<N>(n, [&buffer, &index](uint8_t b) {
            buffer[index++] = b;
        });
        SSASSERT(index == expected.size());
        SSASSERT(memcmp(buffer, expected.data(), index) == 0);
    }

    // Test Deserialize
    {
        Uint32 dN;
        Uint32 index = 0;
        DataDeserializer::DeserializeFromDUI<N>(dN, [&expected, &index](const Uint8** ptr, Uint32 size) {
            if (index + size > expected.size()) {
                *ptr = nullptr;
            }
            *ptr = expected.data() + index;
            index += size;
        });
        SSASSERT(dN == n); // Deserialized should == the original value
    }
}

template <int N>
void GetEncoding(Uint32 n)
{
    printf("DEFINE_DUI_TEST_CASE<%d>(%11u, { ", N, n);
    int i = 0;
    while (n > 0) {
        ++i;

        if (i == N) {
            if (n >> 8 != 0) {
                abort();
            }
            printf("0x%02X, ", n);
            break;
        } else {
            if (n < 128) {
                printf("0x%02X, ", n);
                break;
            } else {
                printf("0x%02X, ", (n & 0x7F) | 0x80);
                n >>= 7u;
            }
        }
        fflush(stdout);
    }
    printf("});\n");
}

static void TestDUI()
{
    // clang-format off
    // GetEncoding<4>(13777276);
    // GetEncoding<4>(152296742);
    // GetEncoding<4>(149702837);
    // GetEncoding<4>(366260920);
    // GetEncoding<4>(148246023);
    // GetEncoding<4>(447253202);
    // GetEncoding<4>(373024602);
    // GetEncoding<4>(226697549);
    // GetEncoding<4>(432443202);
    // GetEncoding<4>(154809111);
    // GetEncoding<4>(496581985);
    // GetEncoding<4>(263705758);
    // GetEncoding<4>(286747438);
    // GetEncoding<4>(386849877);
    // GetEncoding<4>(450978830);
    // GetEncoding<4>(271278562);
    // GetEncoding<4>(102166464);
    // GetEncoding<4>(203950584);
    // GetEncoding<4>(452032335);
    // GetEncoding<4>(165614808);
    // GetEncoding<4>(3043038);
    // GetEncoding<4>(1129192);
    // GetEncoding<4>(4343349);
    // GetEncoding<4>(4217153);
    // GetEncoding<4>(4500600);
    // GetEncoding<4>(345293);
    // GetEncoding<4>(3688492);
    // GetEncoding<4>(2194719);
    // GetEncoding<4>(3858176);
    // GetEncoding<4>(4206262);
    // GetEncoding<4>(3443076);
    // GetEncoding<4>(3434146);
    // GetEncoding<4>(828618);
    // GetEncoding<4>(3742523);
    // GetEncoding<4>(3551655);
    // GetEncoding<4>(621728);
    // GetEncoding<4>(1757688);
    // GetEncoding<4>(4033864);
    // GetEncoding<4>(34433);
    // GetEncoding<4>(3366554);
    DEFINE_DUI_TEST_CASE<4>(   13777276, { 0xFC, 0xF2, 0xC8, 0x06, });
    DEFINE_DUI_TEST_CASE<4>(  152296742, { 0xA6, 0xBA, 0xCF, 0x48, });
    DEFINE_DUI_TEST_CASE<4>(  149702837, { 0xB5, 0x91, 0xB1, 0x47, });
    DEFINE_DUI_TEST_CASE<4>(  366260920, { 0xB8, 0xE5, 0xD2, 0xAE, });
    DEFINE_DUI_TEST_CASE<4>(  148246023, { 0x87, 0x9C, 0xD8, 0x46, });
    DEFINE_DUI_TEST_CASE<4>(  447253202, { 0xD2, 0x95, 0xA2, 0xD5, });
    DEFINE_DUI_TEST_CASE<4>(  373024602, { 0xDA, 0xCE, 0xEF, 0xB1, });
    DEFINE_DUI_TEST_CASE<4>(  226697549, { 0xCD, 0xC2, 0x8C, 0x6C, });
    DEFINE_DUI_TEST_CASE<4>(  432443202, { 0xC2, 0x9E, 0x9A, 0xCE, });
    DEFINE_DUI_TEST_CASE<4>(  154809111, { 0x97, 0xE6, 0xE8, 0x49, });
    DEFINE_DUI_TEST_CASE<4>(  496581985, { 0xE1, 0xFA, 0xE4, 0xEC, });
    DEFINE_DUI_TEST_CASE<4>(  263705758, { 0x9E, 0xA9, 0xDF, 0x7D, });
    DEFINE_DUI_TEST_CASE<4>(  286747438, { 0xAE, 0xD6, 0xDD, 0x88, });
    DEFINE_DUI_TEST_CASE<4>(  386849877, { 0xD5, 0xB8, 0xBB, 0xB8, });
    DEFINE_DUI_TEST_CASE<4>(  450978830, { 0x8E, 0xC8, 0x85, 0xD7, });
    DEFINE_DUI_TEST_CASE<4>(  271278562, { 0xE2, 0xC3, 0xAD, 0x81, });
    DEFINE_DUI_TEST_CASE<4>(  102166464, { 0xC0, 0xDF, 0xDB, 0x30, });
    DEFINE_DUI_TEST_CASE<4>(  203950584, { 0xF8, 0x93, 0xA0, 0x61, });
    DEFINE_DUI_TEST_CASE<4>(  452032335, { 0xCF, 0xEE, 0xC5, 0xD7, });
    DEFINE_DUI_TEST_CASE<4>(  165614808, { 0xD8, 0xA9, 0xFC, 0x4E, });
    DEFINE_DUI_TEST_CASE<4>(    3043038, { 0xDE, 0xDD, 0xB9, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    1129192, { 0xE8, 0xF5, 0x44, });
    DEFINE_DUI_TEST_CASE<4>(    4343349, { 0xB5, 0x8C, 0x89, 0x02, });
    DEFINE_DUI_TEST_CASE<4>(    4217153, { 0xC1, 0xB2, 0x81, 0x02, });
    DEFINE_DUI_TEST_CASE<4>(    4500600, { 0xF8, 0xD8, 0x92, 0x02, });
    DEFINE_DUI_TEST_CASE<4>(     345293, { 0xCD, 0x89, 0x15, });
    DEFINE_DUI_TEST_CASE<4>(    3688492, { 0xAC, 0x90, 0xE1, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    2194719, { 0x9F, 0xFA, 0x85, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    3858176, { 0x80, 0xBE, 0xEB, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    4206262, { 0xB6, 0xDD, 0x80, 0x02, });
    DEFINE_DUI_TEST_CASE<4>(    3443076, { 0x84, 0x93, 0xD2, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    3434146, { 0xA2, 0xCD, 0xD1, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(     828618, { 0xCA, 0xC9, 0x32, });
    DEFINE_DUI_TEST_CASE<4>(    3742523, { 0xBB, 0xB6, 0xE4, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(    3551655, { 0xA7, 0xE3, 0xD8, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(     621728, { 0xA0, 0xF9, 0x25, });
    DEFINE_DUI_TEST_CASE<4>(    1757688, { 0xF8, 0xA3, 0x6B, });
    DEFINE_DUI_TEST_CASE<4>(    4033864, { 0xC8, 0x9A, 0xF6, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(      34433, { 0x81, 0x8D, 0x02, });
    DEFINE_DUI_TEST_CASE<4>(    3366554, { 0x9A, 0xBD, 0xCD, 0x01, });
    DEFINE_DUI_TEST_CASE<4>(         33, { 0x21, });
    DEFINE_DUI_TEST_CASE<4>(        128, { 0x80, 0x01 });

    // GetEncoding<2>(8042);
    // GetEncoding<2>(8778);
    // GetEncoding<2>(17485);
    // GetEncoding<2>(5865);
    // GetEncoding<2>(29624);
    // GetEncoding<2>(5927);
    // GetEncoding<2>(9842);
    // GetEncoding<2>(25206);
    // GetEncoding<2>(21667);
    // GetEncoding<2>(18887);
    // GetEncoding<2>(9734);
    // GetEncoding<2>(31265);
    // GetEncoding<2>(300);
    // GetEncoding<2>(6452);
    // GetEncoding<2>(17045);
    // GetEncoding<2>(7230);
    // GetEncoding<2>(7942);
    // GetEncoding<2>(30001);
    // GetEncoding<2>(31294);
    // GetEncoding<2>(16850);
    DEFINE_DUI_TEST_CASE<2>(       8042, { 0xEA, 0x3E, });
    DEFINE_DUI_TEST_CASE<2>(       8778, { 0xCA, 0x44, });
    DEFINE_DUI_TEST_CASE<2>(      17485, { 0xCD, 0x88, });
    DEFINE_DUI_TEST_CASE<2>(       5865, { 0xE9, 0x2D, });
    DEFINE_DUI_TEST_CASE<2>(      29624, { 0xB8, 0xE7, });
    DEFINE_DUI_TEST_CASE<2>(       5927, { 0xA7, 0x2E, });
    DEFINE_DUI_TEST_CASE<2>(       9842, { 0xF2, 0x4C, });
    DEFINE_DUI_TEST_CASE<2>(      25206, { 0xF6, 0xC4, });
    DEFINE_DUI_TEST_CASE<2>(      21667, { 0xA3, 0xA9, });
    DEFINE_DUI_TEST_CASE<2>(      18887, { 0xC7, 0x93, });
    DEFINE_DUI_TEST_CASE<2>(       9734, { 0x86, 0x4C, });
    DEFINE_DUI_TEST_CASE<2>(      31265, { 0xA1, 0xF4, });
    DEFINE_DUI_TEST_CASE<2>(        300, { 0xAC, 0x02, });
    DEFINE_DUI_TEST_CASE<2>(       6452, { 0xB4, 0x32, });
    DEFINE_DUI_TEST_CASE<2>(      17045, { 0x95, 0x85, });
    DEFINE_DUI_TEST_CASE<2>(       7230, { 0xBE, 0x38, });
    DEFINE_DUI_TEST_CASE<2>(       7942, { 0x86, 0x3E, });
    DEFINE_DUI_TEST_CASE<2>(      30001, { 0xB1, 0xEA, });
    DEFINE_DUI_TEST_CASE<2>(      31294, { 0xBE, 0xF4, });
    DEFINE_DUI_TEST_CASE<2>(      16850, { 0xD2, 0x83, });
    DEFINE_DUI_TEST_CASE<2>(         33, { 0x21, });
    DEFINE_DUI_TEST_CASE<2>(        128, { 0x80, 0x01 });
    DEFINE_DUI_TEST_CASE<2>(      32767, { 0xFF, 0xFF });
    // clang-format on
}

void TestSerializer::test()
{
    std::cout << "Test serialize begin" << std::endl;

    {
        // Test DUI
        TestDUI();
    }

    { // NULL
        Variant null;
        DEFINE_VARIANT_TEST_CASE(null, { 13 });
    }
    { // Integer
        Variant vInt8 = Int8(0x1234567887654321);
        Variant vUint8 = Uint8(0x1234567887654321);
        Variant vInt16 = Int16(0x1234567887654321);
        Variant vUint16 = Uint16(0x1234567887654321);
        Variant vInt32 = Int32(0x1234567887654321);
        Variant vUint32 = Uint32(0x1234567887654321);
        Variant vInt64 = Int64(0x1234567887654321);
        Variant vUint64 = Uint64(0x1234567887654321);

        DEFINE_VARIANT_TEST_CASE(vInt8, { 5, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vUint8, { 6, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vInt16, { 7, 0x43, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vUint16, { 8, 0x43, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vInt32, { 9, 0x87, 0x65, 0x43, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vUint32, { 10, 0x87, 0x65, 0x43, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vInt64, { 11, 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21 });
        DEFINE_VARIANT_TEST_CASE(vUint64, { 12, 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21 });
    }
    { // Array
        auto* arr = new Array(5);
        (*arr)[0] = std::make_shared<Variant>(Uint64(44));
        (*arr)[1] = std::make_shared<Variant>();
        (*arr)[3] = std::make_shared<Variant>(String("hello"));

        Variant vArr(arr);
        DEFINE_VARIANT_TEST_CASE(vArr, {
                                           3, // array
                                           5, // length = 5
                                           12, // first element is uint64
                                           0, 0, 0, 0, 0, 0, 0, 44, // value is 44
                                           13, // the 2nd is null
                                           13, // the 3rd is null
                                           2, // The 4th is string
                                           5, // Length in bytes is 5, DUI
                                           'h', 'e', 'l', 'l', 'o', // Value is "hello"
                                           13, // The 5th is null
                                       });

        auto* arr2 = new Array(130);
        std::vector<uint8_t> result = {
            3, // array
            0x82, 1, // The DUI[4] encoded length: 130
        };
        result.resize(133);
        for (size_t i = 3; i < 133; ++i) {
            result[i] = 13;
        }
        Variant emptyArray(arr2);
        DEFINE_VARIANT_TEST_CASE(emptyArray, result);
    }
    std::cout << "Test serialize pass" << std::endl;
}

}
