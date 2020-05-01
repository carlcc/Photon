//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/PhotonProtocol.h"

namespace pht {

class PhotonProtocol::IProtocolState {
public:
    virtual bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) = 0;
};

// PhotonProtocol State
class PPInitialState : public PhotonProtocol::IProtocol {
public:
    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override
    {
        return false;
    }
};

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