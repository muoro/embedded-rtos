#include "tcp_session.hpp"

void TcpSession::read() {
    auto self = shared_from_this();
    socket_.async_read_some(asio::buffer(input_), [self](auto ec, auto n) {
        if (ec) {
            self->close();
            return;
        }
        self->frames_.feed(
            {self->input_.data(), n},
            [self](const auto& line) {
                if (!self->closed_)
                    self->on_line(line);
            },
            [self] { self->close(); });
        if (!self->closed_)
            self->read();
    });
}

void TcpSession::write() {
    if (closed_ || output_.empty())
        return;
    auto self = shared_from_this();
    auto data = output_.front();
    asio::async_write(socket_, asio::buffer(*data), [self, data](auto ec, auto) {
        if (ec || self->closed_) {
            self->close();
            return;
        }
        self->queued_ -= data->size();
        self->output_.pop_front();
        self->write();
    });
}

TcpSession::TcpSession(asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

void TcpSession::start() {
    read();
}

void TcpSession::send(const std::string& line) {
    if (closed_)
        return;
    if (queued_ + line.size() + 1 > 65536) {
        close();
        return;
    }
    bool idle = output_.empty();
    auto data = std::make_shared<std::string>(line + "\n");
    queued_ += data->size();
    output_.push_back(data);
    if (idle)
        write();
}

void TcpSession::close() {
    if (closed_)
        return;
    closed_ = true;
    asio::error_code ignored;
    socket_.close(ignored);
    on_close();
}
