//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "TestRemoteMethodBinding.h"
#include <SSBase/Convert.h>
#include <photonbase/core/Types.h>
#include <photonbase/protocol/RemoteMethodBinding.h>
#include <photonbase/protocol/RemoteMethodInfo.h>

namespace pht {

namespace internal {

    ReturnValueWrapper<String> PlusIfNotZero(void* context, const String& prefix, Int32 a, const String& suffix)
    {
        if (a == 0) {
            return RemoteMethodException("A should not be 0");
        }
        *(Int32*)context += 1;
        return prefix + ss::Convert::ToString(a) + suffix;
    }
}

void TestRemoteMethodBinding::test()
{
    {
        Int32 context = 1;
        // 1st call: Success
        Array params({
            std::make_shared<Variant>("Hello "),
            std::make_shared<Variant>(Int32(333)),
            std::make_shared<Variant>(" world"),
        });
        RemoteMethodInfo info(Variant::Type::String, "Foo", params);
        RemoteMethodBinding<String(String, Int32, String)> func(&internal::PlusIfNotZero);

        auto ret = func.Invoke(info, &context);
        SSASSERT(ret.first.Get<String>() == "Hello 333 world");
        SSASSERT(ret.second.Is<Null>());
        SSASSERT(context == 2);
    }
    {
        Int32 context = 1;
        // 2nd call: Throw an exception
        Array params({
            std::make_shared<Variant>("Hello "),
            std::make_shared<Variant>(Int32(0)),
            std::make_shared<Variant>(" world"),
        });
        RemoteMethodInfo info(Variant::Type::String, "Foo", params);
        RemoteMethodBinding<String(String, Int32, String)> func(&internal::PlusIfNotZero);

        auto ret = func.Invoke(info, &context);
        SSASSERT(ret.first.Is<Null>());
        SSASSERT(ret.second.Get<String>() == "A should not be 0");
        SSASSERT(context == 1); // context was not increased
    }
    {
        Int32 context = 1;
        // 3rd call: Throw an exception, type mismatch
        Array params({
            std::make_shared<Variant>("Hello "),
            std::make_shared<Variant>(Int32(0)),
            std::make_shared<Variant>(1),
        });
        RemoteMethodInfo info(Variant::Type::String, "Foo", params);
        RemoteMethodBinding<String(String, Int32, String)> func(&internal::PlusIfNotZero);

        auto ret = func.Invoke(info, &context);
        SSASSERT(ret.first.Is<Null>());
        SSASSERT(ret.second.Get<String>() == "Parameter mismatch");
        SSASSERT(context == 1); // context was not increased
    }
}

}
