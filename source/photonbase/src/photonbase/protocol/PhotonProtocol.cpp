//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/protocol/PhotonProtocol.h"

namespace pht {

bool PhotonProtocol::OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    if (memcmp(inputBuffer.GetData<void>(), "bye", 3) == 0) {
        return false;
    }
    inputBuffer.ReadData(outputBuffer, inputBuffer.Size());
    inputBuffer.Reset();
    return true;
}

}