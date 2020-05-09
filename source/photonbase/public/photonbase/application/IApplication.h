//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <set>

namespace pht {

class IProtocol;
class RemoteMethodInfo;

class IApplication {
public:
    virtual bool OnRemoteMethodInvoke(IProtocol* client, const RemoteMethodInfo& method) = 0;

private:
};

}