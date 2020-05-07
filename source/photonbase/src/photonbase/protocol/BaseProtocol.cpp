//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/BaseProtocol.h"

namespace pht {

bool BaseProtocol::OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    return false; // NYI
}

bool BaseProtocol::OnOutBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    return false; // NYI
}

void BaseProtocol::SetHighLevelProtocol(IProtocol* protocol)
{
    highLevelProtocol_ = protocol;
}

void BaseProtocol::SetLowLevelProtocol(IProtocol* protocol)
{
    lowLevelProtocol_ = protocol;
}

void BaseProtocol::SetApplication(IApplication* application)
{
    application_ = application;
}

IProtocol* BaseProtocol::GetHighLevelProtocol() const
{
    return highLevelProtocol_;
}

IProtocol* BaseProtocol::GetLowLevelProtocol() const
{
    return lowLevelProtocol_;
}

IApplication* BaseProtocol::GetApplication() const
{
    return application_;
}

}
