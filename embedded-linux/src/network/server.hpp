#pragma once
#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>

class TcpSession;

// Accepts a single dashboard connection and forwards complete command lines.
class TcpServer {
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<TcpSession> client_;
    void accept();

  public:
    std::function<void(const std::string&)> on_command;
    std::function<void()> on_connect;
    TcpServer(asio::io_context& io, unsigned short port);
    void start();
    void send(const std::string& line);
};
