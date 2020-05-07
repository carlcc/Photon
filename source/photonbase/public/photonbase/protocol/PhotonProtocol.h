//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/protocol/BaseProtocol.h"

namespace pht {

class PhotonProtocol : public BaseProtocol {
public:
    enum class Role {
        kServer,
        kClient
    };
    PhotonProtocol(Role role);
    ~PhotonProtocol();

    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override;

private:
    class IProtocolState;
    class Impl;

    Impl* impl_;
};

}