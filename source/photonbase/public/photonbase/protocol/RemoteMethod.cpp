//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "RemoteMethod.h"

namespace pht {

bool pht::RemoteMethod::MatchPrototype(Variant::Type retType, const String& methodName, const std::vector<Variant::Type>& args)
{
    if (retType != returnType_ || methodName != methodName_ || parameters_.Size() != args.size()) {
        return false;
    }
    auto size = args.size();
    for (size_t i = 0; i < size; ++i) {
        Variant::Type type = parameters_[i] == nullptr ? Variant::Type::Null : parameters_[i]->GetType();
        if (type != args[i]) {
            return false;
        }
    }
    return true;
}

}
