#pragma once
#include "gateway/controller.hpp"
#include "network/server.hpp"
#include "serial/serial_link.hpp"
#include "logging/logger.hpp"
#include <asio.hpp>
#include <cstdint>
#include <string>

struct GatewayOptions {
    std::string serial_device{"/dev/ttyAMA1"};
    unsigned short tcp_port{5556};
    LogLevel log_level{LogLevel::info};
    bool log_stderr{false};

    static GatewayOptions from_arguments(int argc, char** argv);
};

// Composition root: owns the event loop and wires the independent components.
class GatewayApplication {
  public:
    GatewayApplication(GatewayOptions options, Logger& logger);
    void run();

  private:
    GatewayOptions options_;
    Logger& logger_;
    asio::io_context io_;
    TcpServer server_;
    SerialLink serial_;
    room::Controller controller_;
    asio::steady_timer tick_timer_;
    asio::signal_set signals_;
    std::string last_logged_state_;
    std::string last_logged_status_;
    void publish(const std::string& line);

    static std::int64_t now_ms();
    void connect_components();
    void schedule_tick();
    void handle_serial_line(const std::string& line);
};
