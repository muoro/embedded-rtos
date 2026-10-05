#include "GatewayProtocol.hpp"
#include <QMap>
#include <QStringList>

namespace dashboard {
namespace {
using Fields = QMap<QString, QString>;

bool hasBooleans(const Fields& fields, std::initializer_list<const char*> keys) {
    for (const auto* key : keys) {
        if (fields.value(key) != "0" && fields.value(key) != "1") {
            return false;
        }
    }
    return true;
}

bool valueOf(const Fields& fields, const char* key) {
    return fields.value(key) == "1";
}
} // namespace

std::optional<Message> parseMessage(const QString& line) {
    const auto words = line.split(' ', Qt::SkipEmptyParts);
    if (words.size() < 2) {
        return {};
    }
    Fields fields;
    for (int index = 2; index < words.size(); ++index) {
        const auto separator = words[index].indexOf('=');
        const auto key = words[index].left(separator);
        if (separator < 1 || separator + 1 == words[index].size() || fields.contains(key)) {
            return {};
        }
        fields.insert(key, words[index].mid(separator + 1));
    }

    Message message{};
    if (words[0] == "ROOM" && (words[1] == "STATE" || words[1] == "EVENT")) {
        if (!hasBooleans(fields, {"occupied", "light_on", "contact_open"}) ||
            !QStringList{"none", "warning", "alarm"}.contains(fields.value("alarm"))) {
            return {};
        }
        message.kind = Message::Kind::State;
        message.room = {valueOf(fields, "occupied"), valueOf(fields, "light_on"),
                        valueOf(fields, "contact_open"), fields["alarm"]};
    } else if (words[0] == "GATEWAY" && words[1] == "STATUS") {
        if (!hasBooleans(fields, {"online", "valid", "pending", "ready"})) {
            return {};
        }
        message.kind = Message::Kind::Status;
        message.gateway = {valueOf(fields, "online"), valueOf(fields, "valid"),
                           valueOf(fields, "pending"), valueOf(fields, "ready")};
    } else if (words[0] == "GATEWAY" && words[1] == "RESULT") {
        if (!fields.contains("status") || !fields.contains("reason")) {
            return {};
        }
        message.kind = Message::Kind::Result;
        message.result = fields["status"];
        message.reason = fields["reason"];
    } else if (words[0] == "ROOM" && words[1] == "BOOT") {
        if (fields.value("proto") != "1" || fields.value("node").isEmpty()) {
            return {};
        }
        message.kind = Message::Kind::Boot;
    } else if (words[0] == "GATEWAY" && words[1] == "NOTICE") {
        if (!fields.contains("reason")) {
            return {};
        }
        message.kind = Message::Kind::Notice;
        message.reason = fields["reason"];
    } else {
        return {};
    }
    return message;
}

QString lightCommand(bool enabled) {
    return enabled ? "SET light_on 1" : "SET light_on 0";
}
} // namespace dashboard
