//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "IProtocol.h"

namespace pht {

class BaseProtocol : public IProtocol {
public:
    bool OnInBoundData(ss::DynamicBuffer& ioBuffer) override;

    IProtocol* GetHighLevelProtocol() const override;

    IProtocol* GetLowLevelProtocol() const override;

    ITransport* GetTransport() const override;

private:
    IProtocol* highLevelProtocol_ { nullptr };
    IProtocol* lowLevelProtocol_ { nullptr };
};

}