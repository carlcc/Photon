//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "photonbase/protocol/PhotonProtocol.h"
#include "photonbase/application/IApplication.h"
#include "photonbase/protocol/ChunkHeader.h"
#include "photonbase/protocol/DataDeserializer.h"
#include "photonbase/protocol/DataSerializer.h"
#include "photonbase/protocol/MessageHeader.h"
#include "photonbase/protocol/RemoteMethod.h"
#include <set>
#include <unordered_map>

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

struct ChannelContext {
    Uint32 channelId { 0 };
    MessageHeader currentMessageHeader_ {};
    ss::DynamicBuffer messageBuffer_ {};
};

class PhotonProtocol::Impl {
public:
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

    explicit Impl(PhotonProtocol* self, Role role)
    {
        self_ = self;
        if (role == Role::kServer) {
            currentState_ = ProtocolState::kWaitingForHello;
        } else {
            currentState_ = ProtocolState::kInitial;
        }
        channels_[0]; // Create channel 0 by default
    }

    bool ReadChunks(std::set<ChannelContext*>& updatedChannels, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        // unpack as more chunks as possible
        while (!inputBuffer.Empty()) {
            if (ReadingState::kExpectingChunkHeader == readingState_) {
                DataDeserializer deserializer(inputBuffer.GetData<uint8_t>(), inputBuffer.Size());
                if (!deserializer.Deserialize(currentChunkHeader_)) {
                    if (deserializer.IsNotEnoughData()) {
                        break;
                    } else {
                        return false;
                    }
                }
                auto it = channels_.find(currentChunkHeader_.channelId);
                if (it == channels_.end()) {
                    return false; // No such channel
                }

                readingState_ = ReadingState::kExpectingChunkData;
                inputBuffer.Skip(deserializer.DataConsumed());
            } else if (ReadingState::kExpectingChunkHeader == readingState_) {
                if (inputBuffer.Size() < currentChunkHeader_.chunkSize) {
                    // Not enough data
                    break;
                }
                auto it = channels_.find(currentChunkHeader_.channelId);
                if (it == channels_.end()) {
                    return false; // No such channel
                }

                updatedChannels.insert(&it->second);
                it->second.messageBuffer_.PushData(inputBuffer.GetData<Uint8>(), currentChunkHeader_.chunkSize);
                inputBuffer.Skip(currentChunkHeader_.chunkSize);
            }
        }
        return true;
    }

    bool OnRemoteControlMessage(RemoteMethod& rmi, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        return true;
    }

    bool OnRemoteMethodInvoke(RemoteMethod& rmi, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        auto* app = self_->GetApplication();
        if (!app) {

            return false;
        }
        return app->OnRemoteMethodInvoke(self_, rmi);
    }

    bool ReadMessages(ChannelContext& channel, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        auto& msgBuffer = channel.messageBuffer_;
        auto& msgHeader = channel.currentMessageHeader_;
        while (!msgBuffer.Empty()) {
            // Update current message header if necessary.
            if (msgHeader.messageLength == 0) {
                DataDeserializer deserializer(msgBuffer.GetData<Uint8>(), msgBuffer.Size());
                if (!deserializer.Deserialize(msgHeader)) {
                    if (deserializer.IsNotEnoughData()) {
                        // Not enough data, process the next channel
                        break;
                    }
                    return false;
                }
                msgBuffer.Skip(deserializer.DataConsumed());
            }

            // process the message
            if (msgHeader.messageLength > 0) { // messageLength > 0 means we are processing message payloads
                // process the message
                switch (msgHeader.messageType) {
                case MessageHeader::Type::kRemoteMethodInvoke:
                case MessageHeader::Type::kControl: {
                    // ensure the whole message is read, then process the message
                    if (msgBuffer.Size() < msgHeader.messageLength) {
                        return true; // Not enough data
                    }
                    RemoteMethod method;
                    DataDeserializer deserializer(msgBuffer.GetData<Uint8>(), msgBuffer.Size());
                    if (!deserializer.Deserialize(method)) {
                        return false; // We've got enough data, the deserialization ought to be success
                    }
                    if (deserializer.DataConsumed() != msgHeader.messageLength) {
                        return false; // check consistence
                    }
                    msgBuffer.Skip(deserializer.DataConsumed());

                    if (msgHeader.messageType == MessageHeader::Type::kControl) {
                        if (!OnRemoteControlMessage(method, inputBuffer, outputBuffer)) {
                            return false;
                        }
                    } else {
                        if (!OnRemoteMethodInvoke(method, inputBuffer, outputBuffer)) {
                            return false;
                        }
                    }
                    break;
                }
                case MessageHeader::Type::kVideo: {
                    // just forward the whole message payload
                    SSASSERT2(false, "NYI");
                    break;
                }
                case MessageHeader::Type::kAudio: {
                    // just forward the whole message payload
                    SSASSERT2(false, "NYI");
                    break;
                }
                default:
                    SSASSERT2(false, "Not Yet Implemented");
                }
            }
        }
        return true;
    }

    bool OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
    {
        std::set<ChannelContext*> updatedChannels;
        if (!ReadChunks(updatedChannels, inputBuffer, outputBuffer)) {
            return false;
        }

        for (auto* channel : updatedChannels) {
            if (!ReadMessages(*channel, inputBuffer, outputBuffer)) {
                return false;
            }
        }

        return true;
        if (currentState_ == ProtocolState::kWaitingForHello) {

            //
            DataDeserializer deserializer(inputBuffer.GetData<uint8_t>(), inputBuffer.Size());
            RemoteMethod hello;
            if (!deserializer.Deserialize(hello)) {
                // returns true if caused by `not enough data`.
                // else returns false (protocol error)
                return deserializer.IsNotEnoughData();
            }
            if (!hello.MatchPrototype(Variant::Type::Void, "photon.control.hello", {})) {
                // protocol error
                return false;
            }

            inputBuffer.Skip(deserializer.DataConsumed());
            outputBuffer.EnsureSpace(32);
            auto* buffer = outputBuffer.GetEndPtr<Uint8>();
            //            buffer[0] =
        }
    }

    PhotonProtocol* self_ { nullptr };
    ProtocolState currentState_ { ProtocolState::kInvalid };
    ReadingState readingState_ { ReadingState::kExpectingChunkHeader };
    ChunkHeader currentChunkHeader_ {};
    std::unordered_map<Uint32, ChannelContext> channels_;
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