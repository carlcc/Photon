//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "PhotonProtocolImpl.h"
#include "photonbase/application/IApplication.h"
#include "photonbase/protocol/DataDeserializer.h"
#include "photonbase/protocol/RemoteMethod.h"

namespace pht {

PhotonProtocol::Impl::Impl(PhotonProtocol* self, Role role)
{
    self_ = self;
    // TODO: construct a proper handler
    if (role == Role::kServer) {
        currentState_ = ProtocolState::kWaitingForHello;
        protocolHandler_ = nullptr;
    } else {
        currentState_ = ProtocolState::kInitial;
        protocolHandler_ = nullptr;
    }
    channels_[0]; // Create channel 0 by default
}

bool PhotonProtocol::Impl::ReadChunks(std::set<ChannelContext*>& updatedChannels, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
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

bool PhotonProtocol::Impl::OnRemoteMethodInvoke(RemoteMethod& rmi, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    auto* app = self_->GetApplication();
    if (!app) {
        return false;
    }
    return app->OnRemoteMethodInvoke(self_, rmi);
}

class PhotonProtocol::Impl::IProtocolStateDelegate {
public:
    virtual bool ReadMessages(Impl* self, ChannelContext& channel, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) = 0;
};


class PhotonProtocol::Impl::WaitForHelloDelegate : public IProtocolStateDelegate {
public:
    bool ReadMessages(Impl* self, ChannelContext& channel, ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer) override
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

            if (msgHeader.messageType != MessageHeader::Type::kControl // We are expecting 'hello' RMI
                || msgHeader.messageLength == 0 // 'hello' RMI's length > 0
            ) {
                return false;
            }

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

            // TODO process RMI


            // process the message
            if (0 && msgHeader.messageLength > 0) { // messageLength > 0 means we are processing message payloads
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
                        if (!self->OnRemoteControlMessage(method, inputBuffer, outputBuffer)) {
                            return false;
                        }
                    } else {
                        if (!self->OnRemoteMethodInvoke(method, inputBuffer, outputBuffer)) {
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
};

bool PhotonProtocol::Impl::OnInBoundData(ss::DynamicBuffer& inputBuffer, ss::DynamicBuffer& outputBuffer)
{
    std::set<ChannelContext*> updatedChannels;
    if (!ReadChunks(updatedChannels, inputBuffer, outputBuffer)) {
        return false;
    }

    for (auto* channel : updatedChannels) {
        if (!protocolHandler_->ReadMessages(this, *channel, inputBuffer, outputBuffer)) {
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

}
