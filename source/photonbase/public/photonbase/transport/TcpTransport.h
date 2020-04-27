//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <SSBase/Buffer.h>
#include <SSNet/AsyncTcpSocket.h>

namespace pht {

class TcpTransport {
public:
    ss::AsyncTcpSocket* GetSocket() const
    {
        return tcpSocket_;
    }

    void SetSocket(ss::AsyncTcpSocket* s)
    {
        tcpSocket_ = s;
    }

private:
    ss::SharedPtr<ss::AsyncTcpSocket> tcpSocket_ { nullptr };
    ss::DynamicBuffer inputBuffer_ {};
};

}