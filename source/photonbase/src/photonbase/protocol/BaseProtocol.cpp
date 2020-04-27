//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/BaseProtocol.h"

namespace pht {

bool BaseProtocol::OnInBoundData(ss::DynamicBuffer& ioBuffer)
{
    return false; // NYI
}

IProtocol* BaseProtocol::GetHighLevelProtocol() const
{
    return highLevelProtocol_;
}

IProtocol* BaseProtocol::GetLowLevelProtocol() const
{
    return lowLevelProtocol_;
}

IProtocol::ITransport* BaseProtocol::GetTransport() const
{
    return GetLowLevelProtocol()->GetTransport();
}

}
