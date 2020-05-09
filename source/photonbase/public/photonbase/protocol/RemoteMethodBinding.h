//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Variant.h"
#include "photonbase/protocol/RemoteMethodInfo.h"
#include <SSBase/TemplateArgumentCount.h>
#include <utility>

namespace pht {

using RemoteMethodReturnValue = std::pair<Variant, Variant>; // The first is normal return value. If the second is not Variant::Nil, an exception was thrown
struct RemoteMethodException {
    explicit RemoteMethodException(const char* s)
        : what(s)
    {
    }
    explicit RemoteMethodException(String s)
        : what(std::move(s))
    {
    }
    explicit RemoteMethodException(String&& s)
        : what(std::move(s))
    {
    }

    operator RemoteMethodReturnValue() const // NOLINT(google-explicit-constructor)
    {
        return { Variant::Nil, what };
    }

    operator RemoteMethodReturnValue() // NOLINT(google-explicit-constructor)
    {
        return { Variant::Nil, std::move(what) };
    }

    String what;
};

template <class T>
struct ReturnValueWrapper {
    ReturnValueWrapper(RemoteMethodException&& e) // NOLINT(google-explicit-constructor)
    {
        value.first = Variant::Nil;
        value.second = std::move(e.what);
    }
    ReturnValueWrapper(const RemoteMethodException& e) // NOLINT(google-explicit-constructor)
    {
        value.first = Variant::Nil;
        value.second = e.what;
    }
    ReturnValueWrapper(T&& t) // NOLINT(google-explicit-constructor)
    {
        value.first = std::move(t);
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(const T& t) // NOLINT(google-explicit-constructor)
    {
        value.first = t;
        value.second = Variant::Nil;
    }
    operator RemoteMethodReturnValue() const // NOLINT(google-explicit-constructor)
    {
        return value;
    }
    operator RemoteMethodReturnValue() // NOLINT(google-explicit-constructor)
    {
        return std::move(value);
    }
    RemoteMethodReturnValue value;
};
template <>
struct ReturnValueWrapper<String> {
    ReturnValueWrapper(RemoteMethodException&& e) // NOLINT(google-explicit-constructor)
    {
        value.first = Variant::Nil;
        value.second = std::move(e.what);
    }
    ReturnValueWrapper(const RemoteMethodException& e) // NOLINT(google-explicit-constructor)
    {
        value.first = Variant::Nil;
        value.second = e.what;
    }
    ReturnValueWrapper(String&& t) // NOLINT(google-explicit-constructor)
    {
        value.first = std::move(t);
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(const char* t) // NOLINT(google-explicit-constructor)
    {
        value.first = t;
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(const wchar_t* t) // NOLINT(google-explicit-constructor)
    {
        value.first = t;
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(std::string&& t) // NOLINT(google-explicit-constructor)
    {
        value.first = String(t.c_str(), t.length());
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(std::wstring&& t) // NOLINT(google-explicit-constructor)
    {
        value.first = String(t.c_str(), t.length());
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(const std::string& t) // NOLINT(google-explicit-constructor)
    {
        value.first = String(t.c_str(), t.length());
        value.second = Variant::Nil;
    }
    ReturnValueWrapper(const std::wstring& t) // NOLINT(google-explicit-constructor)
    {
        value.first = String(t.c_str(), t.length());
        value.second = Variant::Nil;
    }
    operator RemoteMethodReturnValue() const // NOLINT(google-explicit-constructor)
    {
        return value;
    }
    operator RemoteMethodReturnValue() // NOLINT(google-explicit-constructor)
    {
        return std::move(value);
    }
    RemoteMethodReturnValue value;
};

class IRemoteMethodBinding {
public:
    virtual ~IRemoteMethodBinding() = default;
    virtual RemoteMethodReturnValue Invoke(const RemoteMethodInfo& rmi, void* context) = 0;
};

// TODO: Not sure about this implement's performance
template <class>
class RemoteMethodBinding;

template <class RetType, class... Args>
class RemoteMethodBinding<RetType(Args...)> : public IRemoteMethodBinding {
public:
    static constexpr int ArgumentCount = ss::ArgumentCount<Args...>::Value;

    explicit RemoteMethodBinding(std::function<ReturnValueWrapper<RetType>(void*, Args...)>&& func)
        : func_(std::move(func))
        , parameterTypes_({ Variant::VariantTypeTrait<Args>::TypeEnum... })
        , retType_(Variant::VariantTypeTrait<RetType>::TypeEnum)
    {
    }

    explicit RemoteMethodBinding(const std::function<Variant(Args...)>& func)
        : func_(func_)
        , parameterTypes_({ Variant::VariantTypeTrait<Args>::TypeEnum... })
        , retType_(Variant::VariantTypeTrait<RetType>::TypeEnum)
    {
    }

    RemoteMethodReturnValue Invoke(const RemoteMethodInfo& rmi, void* context) override
    {
        if (ArgumentCount != rmi.GetParameters().Size() // parameter number mismatch
            || rmi.GetReturnType() != retType_ //return type mismatch
        ) {
            return { Variant::Nil, Variant("Parameter or return value mismatch") };
        }

        return InvokeInternal<0>(rmi.GetParameters(), context);
    }

private:
    template <int N, class... ExpandingArgs>
    RemoteMethodReturnValue InvokeInternal(ExpandingArgs&&... args, const Array& arr, void* context)
    {
        if constexpr (N == ArgumentCount) {
            return func_(context, std::forward<Args>(args)...);
        } else {
            Variant nil;
            Variant& v = arr[N] == nullptr ? nil : *arr[N];
            if (v.GetType() != parameterTypes_[N]) {
                return { Variant::Nil, Variant("Parameter mismatch") };
            }
            using Type = typename ss::GetNthType<N, Args...>::Type;
            return InvokeInternal<N + 1, ExpandingArgs..., Type>(std::forward<ExpandingArgs>(args)..., std::forward<Type>(v.Get<Type>()), arr, context);
        }
    }

    std::function<ReturnValueWrapper<RetType>(void*, Args...)> func_;
    std::vector<Variant::Type> parameterTypes_;
    Variant::Type retType_;
};

}