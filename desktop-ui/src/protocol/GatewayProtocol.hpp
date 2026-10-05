#pragma once
#include <QString>
#include <optional>

namespace dashboard {
struct RoomState {
    bool occupied{};
    bool lightOn{};
    bool contactOpen{};
    QString alarm;
};

struct GatewayStatus {
    bool online{};
    bool valid{};
    bool pending{};
    bool ready{};
};

struct Message {
    enum class Kind { State, Status, Result, Boot, Notice };
    Kind kind;
    RoomState room;
    GatewayStatus gateway;
    QString result;
    QString reason;
};

// Parsing has no side effects and can be tested without Qt networking or QML.
std::optional<Message> parseMessage(const QString& line);
QString lightCommand(bool enabled);
} // namespace dashboard
