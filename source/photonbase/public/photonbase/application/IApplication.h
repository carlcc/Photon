//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <set>

namespace pht {

class IProtocol;
class RemoteMethod;

class IApplication {
public:
    virtual bool OnRemoteMethodInvoke(IProtocol* client, const RemoteMethod& method) = 0;

private:
};

}