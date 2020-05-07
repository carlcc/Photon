//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "IApplication.h"
#include <set>

namespace pht {

class IProtocol;

class BaseApplication : public IApplication {
public:
    const std::set<IProtocol*>& GetClients() const;


private:
    std::set<IProtocol*> clients_;
};

}