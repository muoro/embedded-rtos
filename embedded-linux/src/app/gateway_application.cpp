#include "gateway_application.hpp"
#include <charconv>
#include <csignal>
#include <stdexcept>
#include <utility>

GatewayOptions GatewayOptions::from_arguments(int argc, char** argv) {
    GatewayOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string option = argv[index];
        if (option == "--serial" && index + 1 < argc) {
            options.serial_device = argv[++index];
        } else if (option == "--port" && index + 1 < argc) {
            const std::string value = argv[++index];
            unsigned int port = 0;
            const auto parsed = std::from_chars(value.data(), value.data() + value.size(), port);
            if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() ||
                port == 0 || port > 65535) {
                throw std::runtime_error("Invalid TCP port: " + value);
            }
            options.tcp_port = static_cast<unsigned short>(port);
        } else if (option == "--log-level" && index + 1 < argc) {
            options.log_level = Logger::parse_level(argv[++index]);
        } else if (option == "--log-stderr") {
            options.log_stderr = true;
        } else {
            throw std::runtime_error("Usage: device-gateway [--serial PATH] [--port PORT] "
                                     "[--log-level error|warning|info|debug] [--log-stderr]");
        }
    }
    return options;
}

GatewayApplication::GatewayApplication(GatewayOptions options, Logger& logger)
    : options_(std::move(options)), logger_(logger), server_(io_, options_.tcp_port),
      serial_(io_, options_.serial_device),
      controller_([this](const auto& line) {
                      logger_.write(LogLevel::debug, "UART TX: " + line);
                      serial_.send(line);
                  },
                  [this](const auto& line) { publish(line); }),
      tick_timer_(io_), signals_(io_, SIGINT, SIGTERM) {
    connect_components();
}

void GatewayApplication::connect_components() {
    serial_.on_line = [this](const auto& line) { handle_serial_line(line); };
    serial_.on_error = [this](const auto& message) {
        logger_.write(LogLevel::error, "UART: " + message);
        controller_.disconnect("serial_error");
    };
    serial_.on_open = [this] {
        logger_.write(LogLevel::info, "UART opened: " + options_.serial_device);
        controller_.start(now_ms());
    };
    server_.on_command = [this](const auto& line) { controller_.command(line, now_ms()); };
    server_.on_connect = [this] {
        logger_.write(LogLevel::info, "TCP client connected");
        controller_.snapshot();
    };
    signals_.async_wait([this](auto, auto) { io_.stop(); });
}

void GatewayApplication::run() {
    server_.start();
    serial_.open();
    schedule_tick();
    logger_.write(LogLevel::info, "Started: " + options_.serial_device + " <-> TCP " +
                                   std::to_string(options_.tcp_port));
    io_.run();
    logger_.write(LogLevel::info, "Stopped");
}

void GatewayApplication::publish(const std::string& line) {
    if (line.rfind("GATEWAY STATUS", 0) == 0 && line != last_logged_status_) {
        logger_.write(line.find("online=0") != std::string::npos ? LogLevel::warning : LogLevel::info, line);
        last_logged_status_ = line;
    } else if (line.rfind("GATEWAY RESULT", 0) == 0 || line.rfind("GATEWAY NOTICE", 0) == 0) {
        const bool normal = line.find("status=pending") != std::string::npos ||
                            line.find("status=confirmed") != std::string::npos;
        logger_.write(normal ? LogLevel::info : LogLevel::warning, line);
    }
    server_.send(line);
}

void GatewayApplication::schedule_tick() {
    tick_timer_.expires_after(std::chrono::milliseconds(250));
    tick_timer_.async_wait([this](auto error) {
        if (!error) {
            controller_.tick(now_ms());
            schedule_tick();
        }
    });
}

void GatewayApplication::handle_serial_line(const std::string& line) {
    logger_.write(LogLevel::debug, "UART RX: " + line);
    const auto message = room::parse(line);
    if (message && message->kind != "PONG") {
        if (message->kind != "STATE" || line != last_logged_state_) {
            logger_.write(message->kind == "ERR" ? LogLevel::warning : LogLevel::info, line);
        }
        if (message->kind == "STATE") {
            last_logged_state_ = line;
        }
    }
    controller_.receive(line, now_ms());
}

std::int64_t GatewayApplication::now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
