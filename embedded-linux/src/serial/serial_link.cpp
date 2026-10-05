#include "serial_link.hpp"
#include <sys/file.h>
#include <termios.h>

void SerialLink::fail(const std::string& reason) {
    asio::error_code ignored;
    port_.cancel(ignored);
    port_.close(ignored);
    ++generation_;
    output_.clear();
    frames_.reset();
    on_error(reason);
    if (!stopping_) {
        retry_.expires_after(std::chrono::seconds(2));
        retry_.async_wait([this](auto ec) {
            if (!ec)
                open();
        });
    }
}

void SerialLink::read() {
    auto generation = generation_;
    port_.async_read_some(asio::buffer(input_), [this, generation](auto ec, auto n) {
        if (generation != generation_)
            return;
        if (ec) {
            fail(ec.message());
            return;
        }
        frames_.feed({input_.data(), n}, on_line, [this] { on_error("UART frame too long"); });
        read();
    });
}

void SerialLink::write() {
    if (output_.empty())
        return;
    auto data = output_.front();
    auto generation = generation_;
    asio::async_write(port_, asio::buffer(*data), [this, data, generation](auto ec, auto) {
        if (generation != generation_)
            return;
        if (ec) {
            fail(ec.message());
            return;
        }
        output_.pop_front();
        write();
    });
}

SerialLink::SerialLink(asio::io_context& io, std::string path)
    : port_(io), retry_(io), path_(std::move(path)) {}

void SerialLink::open() {
    asio::error_code ec;
    port_.open(path_, ec);
    if (ec) {
        fail(ec.message());
        return;
    }
    if (flock(port_.native_handle(), LOCK_EX | LOCK_NB) != 0) {
        fail("UART already owned");
        return;
    }
    termios options{};
    if (tcgetattr(port_.native_handle(), &options) != 0) {
        fail("tcgetattr failed");
        return;
    }
    cfmakeraw(&options);
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;
    if (tcsetattr(port_.native_handle(), TCSANOW, &options) != 0) {
        fail("tcsetattr failed");
        return;
    }
    tcflush(port_.native_handle(), TCIFLUSH);
    read();
    on_open();
}

void SerialLink::send(const std::string& line) {
    if (!port_.is_open())
        return;
    if (output_.size() >= 32) {
        fail("UART output queue full");
        return;
    }
    bool idle = output_.empty();
    output_.push_back(std::make_shared<std::string>(line + "\n"));
    if (idle)
        write();
}
