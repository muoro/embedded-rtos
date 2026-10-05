#pragma once
#include "protocol/line_framer.hpp"
#include <array>
#include <asio.hpp>
#include <deque>
#include <functional>
#include <memory>

// Owns one serial device and its asynchronous read/write queues.
class SerialLink {
    asio::serial_port port_;
    asio::steady_timer retry_;
    std::string path_;
    std::array<char, 512> input_{};
    room::LineFramer frames_;
    std::deque<std::shared_ptr<std::string>> output_;
    std::size_t generation_{0};
    bool stopping_{false};
    void fail(const std::string& reason);
    void read();
    void write();

  public:
    std::function<void(const std::string&)> on_line = [](auto&) {};
    std::function<void(const std::string&)> on_error = [](auto&) {};
    std::function<void()> on_open = [] {};
    SerialLink(asio::io_context& io, std::string path);
    void open();
    void send(const std::string& line);
};
