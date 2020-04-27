//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <SSBase/Buffer.h>

namespace pht {

class TcpTransport;

class IProtocol {
public:
    // TODO Have no idea about what an ITransport interface should be look like, use TcpTransport now
    using ITransport = TcpTransport;

    virtual bool OnInBoundData(ss::DynamicBuffer& ioBuffer) = 0;

    virtual IProtocol* GetHighLevelProtocol() const = 0;

    virtual IProtocol* GetLowLevelProtocol() const = 0;

    virtual ITransport* GetTransport() const = 0;
};

}