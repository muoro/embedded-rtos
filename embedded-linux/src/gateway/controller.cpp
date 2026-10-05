#include "controller.hpp"
#include <utility>

namespace room {
Controller::Controller(Sink uart, Sink client)
    : send_uart_(std::move(uart)), send_client_(std::move(client)) {}

void Controller::start(std::int64_t now) {
    last_ping_at_ = now;
    send_uart_("PING");
    request_state();
    publish_status();
}

void Controller::tick(std::int64_t now) {
    if (online_ && now - last_received_at_ >= device_timeout_ms) {
        disconnect("device_timeout");
    }
    if (pending_ && now - pending_->started_at >= command_timeout_ms) {
        expire_command(now);
    }
    if (now - last_ping_at_ >= heartbeat_interval_ms) {
        last_ping_at_ = now;
        send_uart_("PING");
        if (!valid_) {
            request_state();
        }
    }
}

void Controller::receive(const std::string& line, std::int64_t now) {
    const auto message = parse(line);
    if (!message) {
        return; // Malformed messages must not keep the device online.
    }

    const bool was_online = online_;
    online_ = true;
    last_received_at_ = now;

    if (message->kind == "BOOT") {
        handle_boot();
    } else if (!was_online) {
        valid_ = false;
        request_state();
    }

    send_client_(line);
    update_state(*message);
    handle_reply(*message, now);

    if (now >= recovery_until_ && valid_) {
        recovery_until_ = 0;
    }
    publish_status();
}

void Controller::command(const std::string& line, std::int64_t now) {
    if (line == "GET STATE") {
        snapshot();
        request_state();
        return;
    }
    if (line != "SET light_on 0" && line != "SET light_on 1") {
        publish_result("error", "invalid_command");
        return;
    }
    if (!online_ || !valid_) {
        publish_result("offline", "state_unavailable");
        return;
    }
    if (!ready()) {
        publish_result("busy", "resynchronizing");
        return;
    }

    pending_ = PendingCommand{line.back() == '1', false, now};
    publish_result("pending", "awaiting_device");
    publish_status();
    send_uart_(line);
}

void Controller::handle_boot() {
    valid_ = false;
    if (pending_) {
        pending_.reset();
        publish_result("error", "device_reset");
    }
    recovery_until_ = 0;
    request_state();
}

void Controller::update_state(const Message& message) {
    if (!message.state) {
        return;
    }
    state_ = message.state;
    valid_ = true;

    // Only a STATE requested after ACK can confirm the pending command.
    if (pending_ && pending_->acknowledged && message.kind == "STATE") {
        const bool accepted = state_->light_on == pending_->requested_light;
        pending_.reset();
        publish_result(accepted ? "confirmed" : "rejected",
                       accepted ? "state_verified" : "device_rule");
    }
}

void Controller::handle_reply(const Message& message, std::int64_t now) {
    if (!pending_) {
        return;
    }
    if (message.kind == "ACK" && message.fields.at("property") == "light_on") {
        pending_->acknowledged = true;
        request_state();
    } else if (message.kind == "ERR") {
        pending_.reset();
        valid_ = false;
        recovery_until_ = now + recovery_interval_ms;
        publish_result("error", message.fields.at("code"));
        request_state();
    }
}

void Controller::expire_command(std::int64_t now) {
    pending_.reset();
    valid_ = false;
    recovery_until_ = now + recovery_interval_ms;
    publish_result("timeout", "outcome_unknown");
    publish_status();
    request_state(); // Query state, never retry the original command.
}

void Controller::disconnect(const std::string& reason) {
    if (pending_) {
        pending_.reset();
        publish_result("error", "outcome_unknown");
    }
    online_ = false;
    valid_ = false;
    recovery_until_ = 0;
    publish_status();
    send_client_("GATEWAY NOTICE reason=" + reason);
}

void Controller::snapshot() {
    if (state_) {
        send_client_(format(*state_));
    }
    publish_status();
}

bool Controller::ready() const {
    return online_ && valid_ && !pending_ && recovery_until_ == 0;
}

std::string Controller::status_line() const {
    return "GATEWAY STATUS online=" + std::to_string(online_) + " valid=" + std::to_string(valid_) +
           " pending=" + std::to_string(pending_.has_value()) + " ready=" + std::to_string(ready());
}

void Controller::request_state() {
    send_uart_("GET STATE");
}
void Controller::publish_status() {
    send_client_(status_line());
}

void Controller::publish_result(const std::string& status, const std::string& reason) {
    send_client_("GATEWAY RESULT status=" + status + " reason=" + reason);
}
} // namespace room
