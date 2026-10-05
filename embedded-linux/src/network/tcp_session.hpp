#pragma once
#include "protocol/line_framer.hpp"
#include <array>
#include <asio.hpp>
#include <deque>
#include <memory>

class TcpSession : public std::enable_shared_from_this<TcpSession> {
    asio::ip::tcp::socket socket_;
    std::array<char, 1024> input_{};
    room::LineFramer frames_;
    std::deque<std::shared_ptr<std::string>> output_;
    std::size_t queued_{0};
    bool closed_{false};
    void read();
    void write();

  public:
    std::function<void(const std::string&)> on_line;
    std::function<void()> on_close = [] {};
    explicit TcpSession(asio::ip::tcp::socket socket);
    void start();
    void send(const std::string& line);
    void close();
};
