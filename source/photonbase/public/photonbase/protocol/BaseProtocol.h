//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "IProtocol.h"

namespace pht {

class BaseProtocol : public IProtocol {
public:
    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override;

    bool OnOutBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override;

    void SetHighLevelProtocol(IProtocol* protocol);

    void SetLowLevelProtocol(IProtocol* protocol);

    IProtocol* GetHighLevelProtocol() const override;

    IProtocol* GetLowLevelProtocol() const override;

private:
    IProtocol* highLevelProtocol_ { nullptr };
    IProtocol* lowLevelProtocol_ { nullptr };
};

}