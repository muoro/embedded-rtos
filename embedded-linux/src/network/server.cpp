#include "server.hpp"
#include "tcp_session.hpp"

void TcpServer::accept() {
    acceptor_.async_accept([this](auto ec, auto socket) {
        if (!ec) {
            if (client_) {
                // v1 deliberately supports one controller.
                asio::error_code ignored;
                socket.close(ignored);
            } else {
                client_ = std::make_shared<TcpSession>(std::move(socket));
                client_->on_line = [this](const auto& line) { on_command(line); };
                client_->on_close = [this] { client_.reset(); };
                client_->start();
                on_connect();
            }
        }
        if (acceptor_.is_open())
            accept();
    });
}

TcpServer::TcpServer(asio::io_context& io, unsigned short port) : acceptor_(io) {
    acceptor_.open(asio::ip::tcp::v4());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind({asio::ip::tcp::v4(), port});
    acceptor_.listen();
}

void TcpServer::start() {
    accept();
}

void TcpServer::send(const std::string& line) {
    if (client_) {
        auto client = client_;
        client->send(line);
    }
}
