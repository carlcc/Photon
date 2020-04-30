//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <SSBase/Buffer.h>

namespace pht {

class IProtocol {
public:

    virtual bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) = 0;

    virtual bool OnOutBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) = 0;

    virtual IProtocol* GetHighLevelProtocol() const = 0;

    virtual IProtocol* GetLowLevelProtocol() const = 0;
};

}