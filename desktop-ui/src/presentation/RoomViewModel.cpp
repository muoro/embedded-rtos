#include "RoomViewModel.hpp"

RoomViewModel::RoomViewModel(QObject* parent) : QObject(parent) {
    connect(&connection_, &GatewayConnection::connectionChanged, this,
            &RoomViewModel::handleConnection);
    connect(&connection_, &GatewayConnection::lineReceived, this, &RoomViewModel::handleLine);
}

void RoomViewModel::start(const QString& host, quint16 port) {
    connection_.start(host, port);
}
bool RoomViewModel::connected() const {
    return connection_.isConnected();
}
bool RoomViewModel::online() const {
    return status_.online;
}
bool RoomViewModel::valid() const {
    return status_.valid;
}
bool RoomViewModel::pending() const {
    return status_.pending;
}
bool RoomViewModel::ready() const {
    return connected() && status_.online && status_.valid && status_.ready && !status_.pending;
}
QVariantMap RoomViewModel::room() const {
    return room_;
}
QString RoomViewModel::result() const {
    return result_;
}
EventListModel* RoomViewModel::events() {
    return &events_;
}

void RoomViewModel::handleConnection(bool isConnected) {
    if (isConnected) {
        recordEvent("LINK", "Connected to Linux gateway");
        refresh();
    } else {
        if (status_.pending) {
            result_ = "Unknown — connection lost";
        }
        const bool hadState = status_.online || status_.valid;
        status_ = {};
        if (hadState) {
            recordEvent("LINK", "Disconnected; cached values are stale");
        }
    }
    emit changed();
}

void RoomViewModel::refresh() {
    if (connected()) {
        connection_.sendLine("GET STATE");
        emit trafficObserved("GET STATE", true);
    }
}

void RoomViewModel::setLight(bool enabled) {
    if (!ready()) {
        return;
    }
    status_.pending = true;
    result_ = "Pending — awaiting device";
    connection_.sendLine(dashboard::lightCommand(enabled));
    emit trafficObserved(dashboard::lightCommand(enabled), true);
    recordEvent("CMD", enabled ? "Requested light On" : "Requested light Off");
    emit changed();
}

void RoomViewModel::handleLine(const QString& line) {
    const auto message = dashboard::parseMessage(line);
    if (!message) {
        return;
    }
    emit trafficObserved(line, false);
    using Kind = dashboard::Message::Kind;
    switch (message->kind) {
    case Kind::State:
        applyState(message->room);
        break;
    case Kind::Status:
        if (message->gateway.online != status_.online) {
            recordEvent("LINK", message->gateway.online ? "Device connected" : "Device offline");
        }
        status_ = message->gateway;
        break;
    case Kind::Result:
        applyResult(*message);
        break;
    case Kind::Boot:
        status_.valid = false;
        status_.ready = false;
        recordEvent("BOOT", "Device restarted; requesting state");
        break;
    case Kind::Notice:
        recordEvent("LINK", message->reason);
        break;
    }
    emit changed();
}

void RoomViewModel::applyState(const dashboard::RoomState& state) {
    const QVariantMap next{{"occupied", state.occupied},
                           {"light_on", state.lightOn},
                           {"contact_open", state.contactOpen},
                           {"alarm", state.alarm}};
    if (room_.isEmpty()) {
        recordEvent("STATE", "Received initial room state");
    } else {
        if (next["occupied"] != room_["occupied"])
            recordEvent("STATE", state.occupied ? "Room became occupied" : "Room became vacant");
        if (next["light_on"] != room_["light_on"])
            recordEvent("STATE", state.lightOn ? "Light changed to On" : "Light changed to Off");
        if (next["contact_open"] != room_["contact_open"])
            recordEvent("STATE", state.contactOpen ? "Contact opened" : "Contact closed");
        if (next["alarm"] != room_["alarm"])
            recordEvent("STATE", "Alarm changed to " + state.alarm);
    }
    room_ = next;
}

void RoomViewModel::applyResult(const dashboard::Message& message) {
    const auto& value = message.result;
    if (value == "pending") {
        result_ = "Pending — awaiting device";
    } else if (value == "confirmed") {
        result_ = "Confirmed — device state verified";
    } else if (value == "rejected") {
        result_ = "Rejected — room rule keeps the light on";
    } else if (value == "timeout") {
        result_ = "Timeout — outcome unknown";
    } else {
        result_ = value + " — " + message.reason;
    }
    status_.pending = value == "pending";
    recordEvent(value == "confirmed" ? "ACK" : value == "pending" ? "CMD" : "WARN", result_);
}

void RoomViewModel::recordEvent(const QString& category, const QString& text) {
    events_.append(category, text);
}

RoomViewModel::~RoomViewModel() {
    disconnect(&connection_, nullptr, this, nullptr);
}
