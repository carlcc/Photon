//
// Created by carl on 2020/4/30 0030.
//

#pragma once

#include <SSNet/AsyncTcpSocket.h>
#include <photonbase/protocol/PhotonProtocol.h>
#include <spdlog/spdlog.h>

namespace phtserver {

class ClientHandle {
public:
    explicit ClientHandle(ss::AsyncTcpSocket* socket)
    {
        peer_ = socket->GetPeer();
        protocol_ = std::make_unique<pht::PhotonProtocol>();
        socket_ = socket;
        SPDLOG_DEBUG("ClientHandle for {}:{} constructed", peer_.IP().ToStdString(), peer_.Port());
    }

    ~ClientHandle()
    {
        SPDLOG_DEBUG("Client handle for {}:{} destroyed", peer_.IP().ToStdString(), peer_.Port());
    }

    void OnClientData(ssize_t nread, const char* data)
    {
        if (nread < 0) {
            // TODO handle error code
            SPDLOG_INFO("Got nread {}", nread);
            socket_->Close(nullptr);
            return;
        }

        if (nread == 0) {
            return; // ignore, this may caused by signals
        }

        SPDLOG_INFO("Receive {} bytes", nread);
        if (protocol_ != nullptr) {
            inputBuffer_.PushData(data, uint32_t(nread));

            if (!protocol_->OnInBoundData(inputBuffer_, outputBuffer_)) {
                Close();
                return;
            }
            if (outputBuffer_.Empty()) {
                return;
            }

            auto ret = OnOutBoundData(outputBuffer_.GetData<void>(), outputBuffer_.Size());
            if (ret < 0) {
                Close();
                return;
            }

            outputBuffer_.Reset();
        }
    }

    int OnOutBoundData(const void* data, uint32_t len)
    {
        if (len > 0) {
            int ret = socket_->Send(data, len, [this](int status) {
                if (status != 0) {
                    Close();
                }
            });
            // TODO manipulate the ret code
            return ret;
        }
        return 0;
    }

    void Close()
    {
        socket_->Close(nullptr);
    }

private:
    std::unique_ptr<pht::IProtocol> protocol_ { nullptr };
    ss::AsyncTcpSocket* socket_ { nullptr };
    ss::EndPoint peer_ {};
    ss::DynamicBuffer inputBuffer_ {};
    ss::DynamicBuffer outputBuffer_ {};
};

}
