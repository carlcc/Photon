//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/protocol/BaseProtocol.h"

namespace pht {

class PhotonProtocol : public BaseProtocol {
public:

    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override;
};

}