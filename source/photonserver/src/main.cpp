//
// Created by carl on 20-3-31.
//

#include "spdlog/spdlog.h"
#include <SSNet/AsyncTcpSocket.h>
#include <SSNet/Loop.h>
#include <iostream>

void OnClientData(const ss::SharedPtr<ss::AsyncTcpSocket>& client, ssize_t nread, const char* data)
{
    if (nread < 0) {
        SPDLOG_INFO("Got nread {}", nread);
        client->Close(nullptr);
        return;
    }

    SPDLOG_INFO("Receive {} bytes", nread);
    client->Send(data, nread, [client](int status) {
        if (status != 0) {
            SPDLOG_WARN("Send data failed");
        }
        client->Close(nullptr);
    });
}

void OnConnection(const ss::SharedPtr<ss::AsyncTcpSocket>& server, int status)
{
    if (status != 0) {
        SPDLOG_WARN("OnConnection got status {}", status);
        return;
    }
    auto client = server->Accept();
    if (client == nullptr) {
        SPDLOG_WARN("Accept client failed!");
        return;
    }
    SPDLOG_INFO("A client accepted");
    // TODO create a TcpTransport

    client->StartReceive([client](ssize_t nread, const char* data) {
        OnClientData(client, nread, data);
    });
}

void ConfigureLog()
{
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    spdlog::set_pattern("[%H:%M:%S %z|%t|%l|%s:%#] %v");
}

int main()
{
    ConfigureLog();

    auto loop = ss::MakeShared<ss::Loop>();
    auto server = loop->CreateTcpSocket();

    // TODO: handle signals

    // TODO: read from configure file
    const char* ip = "0.0.0.0";
    uint16_t port = 6666;
    int backlog = 100;

    if (0 != server->Bind("0.0.0.0", 6666)) {
        SPDLOG_INFO("Start listening: {}:{}", ip, port);
        return -1;
    }
    SPDLOG_INFO("Bind to: {}:{}", ip, port);

    auto ret = server->Listen(backlog, [](ss::AsyncTcpSocket* server, int status) {
        OnConnection(server, status);
    });

    if (0 != ret) {
        SPDLOG_INFO("Listening on: {}:{} failed", ip, port);
        return -1;
    }
    SPDLOG_INFO("Listening on: {}:{}", ip, port);

    loop->Run();

    return 0;
}