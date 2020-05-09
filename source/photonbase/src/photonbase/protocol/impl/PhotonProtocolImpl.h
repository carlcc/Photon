//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"
#include "photonbase/protocol/ChunkHeader.h"
#include "photonbase/protocol/MessageHeader.h"
#include "photonbase/protocol/PhotonProtocol.h"
#include <set>
#include <unordered_map>

namespace pht {

class RemoteMethod;
class IApplication;

struct ChannelContext {
    Uint32 channelId { 0 };
    MessageHeader currentMessageHeader_ {};
    ss::DynamicBuffer messageBuffer_ {};
};


class PhotonProtocol::Impl {
public:
    class IProtocolStateDelegate;
    class WaitForHelloDelegate;

    enum class ProtocolState {
        kInvalid,
        kInitial, // client
        kWaitingForHello, // server
        kWaitingForVersionList, // server
        kWaitingForHelloReply, // client
        kWaitingForVersionSelected, // client
    };
    enum class ReadingState {
        kExpectingChunkHeader,
        kExpectingChunkData
    };

    explicit Impl(PhotonProtocol* self, Role role);

    bool ReadChunks(std::set<ChannelContext*>& updatedChannels, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer);

    bool OnRemoteControlMessage(RemoteMethod& rmi, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        return true;
    }

    bool OnRemoteMethodInvoke(RemoteMethod& rmi, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer);

    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer);

private:
    PhotonProtocol* self_ { nullptr };
    ProtocolState currentState_ { ProtocolState::kInvalid };
    IProtocolStateDelegate* protocolHandler_ { nullptr };
    ReadingState readingState_ { ReadingState::kExpectingChunkHeader };
    ChunkHeader currentChunkHeader_ {};
    std::unordered_map<Uint32, ChannelContext> channels_;
};

}