//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Variant.h"
#include <vector>

namespace pht {

class RemoteMethodInfo {
public:
    RemoteMethodInfo() = default;

    RemoteMethodInfo(Variant::Type retType, String&& methodName, Array&& args)
        : methodName_(std::move(methodName))
        , returnType_(retType)
        , parameters_(std::move(args))
    {
    }

    RemoteMethodInfo(Variant::Type retType, const String& methodName, Array&& args)
        : methodName_(methodName)
        , returnType_(retType)
        , parameters_(std::move(args))
    {
    }

    RemoteMethodInfo(Variant::Type retType, String&& methodName, const Array& args)
        : methodName_(std::move(methodName))
        , returnType_(retType)
        , parameters_(std::move(args))
    {
    }

    RemoteMethodInfo(Variant::Type retType, const String& methodName, const Array& args)
        : methodName_(methodName)
        , returnType_(retType)
        , parameters_(std::move(args))
    {
    }

    bool MatchPrototype(Variant::Type retType, const String& methodName, const std::vector<Variant::Type>& args);

    const String& GetMethodName() const
    {
        return methodName_;
    }

    Variant::Type GetReturnType() const
    {
        return returnType_;
    }

    const Array& GetParameters() const
    {
        return parameters_;
    }

    Array& GetParameters()
    {
        return parameters_;
    }

    void SetMethodName(String&& methodName)
    {
        methodName_ = std::forward<String>(methodName);
    }

    void SetReturnType(Variant::Type returnType)
    {
        returnType_ = returnType;
    }

    void SetParameters(Array&& parameters)
    {
        parameters_ = std::move(parameters);
    }

    void SetParameters(const Array& parameters)
    {
        parameters_ = parameters;
    }

private:
    String methodName_ {};
    Variant::Type returnType_ { Variant::Type::Void };
    Array parameters_ {};

    friend class DataDeserializer;
    friend class DataSerializer;
};

}