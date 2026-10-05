#pragma once
#include "protocol/protocol.hpp"
#include <cstdint>
#include <functional>
#include <optional>

namespace room {
// Device state machine. It knows no sockets, serial ports or UI objects.
// The caller supplies time and output functions, making behavior testable.
class Controller {
  public:
    using Sink = std::function<void(const std::string&)>;
    Controller(Sink uart, Sink client);

    void start(std::int64_t now);
    void tick(std::int64_t now);
    void receive(const std::string& line, std::int64_t now);
    void command(const std::string& line, std::int64_t now);
    void disconnect(const std::string& reason);
    void snapshot();
    std::string status_line() const;

  private:
    static constexpr std::int64_t heartbeat_interval_ms = 2000;
    static constexpr std::int64_t device_timeout_ms = 6000;
    static constexpr std::int64_t command_timeout_ms = 3000;
    static constexpr std::int64_t recovery_interval_ms = 3000;

    struct PendingCommand {
        bool requested_light;
        bool acknowledged;
        std::int64_t started_at;
    };

    Sink send_uart_;
    Sink send_client_;
    bool online_{false};
    bool valid_{false};
    std::int64_t last_received_at_{0};
    std::int64_t last_ping_at_{0};
    std::int64_t recovery_until_{0};
    std::optional<State> state_;
    std::optional<PendingCommand> pending_;

    bool ready() const;
    void request_state();
    void publish_status();
    void publish_result(const std::string& status, const std::string& reason);
    void handle_boot();
    void update_state(const Message& message);
    void handle_reply(const Message& message, std::int64_t now);
    void expire_command(std::int64_t now);
};
} // namespace room
