//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/PhotonProtocol.h"
#include "impl/PhotonProtocolImpl.h"
#include "photonbase/application/IApplication.h"
#include "photonbase/protocol/ChunkHeader.h"
#include "photonbase/protocol/DataDeserializer.h"
#include "photonbase/protocol/DataSerializer.h"
#include "photonbase/protocol/MessageHeader.h"
#include "photonbase/protocol/RemoteMethodInfo.h"
#include <set>

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

PhotonProtocol::PhotonProtocol(Role role)
    : impl_(new Impl(this, role))
{
}

PhotonProtocol::~PhotonProtocol()
{
    delete impl_;
}

bool PhotonProtocol::OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    // return impl_->OnInBoundData(inputBuffer, outputBuffer);
    if (memcmp(inputBuffer.GetData<void>(), "bye", 3) == 0) {
        return false;
    }
    inputBuffer.ReadData(outputBuffer, inputBuffer.Size());
    inputBuffer.Reset();
    return true;
}

}