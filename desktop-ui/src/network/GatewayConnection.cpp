#include "GatewayConnection.hpp"

GatewayConnection::GatewayConnection(QObject* parent) : QObject(parent) {
    reconnectTimer_.setInterval(reconnectIntervalMs);
    connect(&reconnectTimer_, &QTimer::timeout, this, &GatewayConnection::connectNow);
    connect(&socket_, &QTcpSocket::connected, this, [this] {
        input_.clear();
        emit connectionChanged(true);
    });
    connect(&socket_, &QTcpSocket::disconnected, this, &GatewayConnection::handleDisconnect);
    connect(&socket_, &QTcpSocket::errorOccurred, this, [this](auto) { handleDisconnect(); });
    connect(&socket_, &QTcpSocket::readyRead, this, &GatewayConnection::receiveBytes);
}

void GatewayConnection::start(const QString& host, quint16 port) {
    host_ = host;
    port_ = port;
    reconnectTimer_.start();
    connectNow();
}

bool GatewayConnection::isConnected() const {
    return socket_.state() == QAbstractSocket::ConnectedState;
}

void GatewayConnection::connectNow() {
    if (socket_.state() == QAbstractSocket::UnconnectedState) {
        socket_.connectToHost(host_, port_);
    }
}

void GatewayConnection::sendLine(const QString& line) {
    if (isConnected()) {
        socket_.write(line.toUtf8() + '\n');
    }
}

void GatewayConnection::handleDisconnect() {
    input_.clear();
    emit connectionChanged(false);
}

void GatewayConnection::receiveBytes() {
    input_ += socket_.readAll();
    while (true) {
        const auto newline = input_.indexOf('\n');
        if (newline < 0) {
            break;
        }
        if (newline > maximumLineBytes) {
            socket_.abort();
            return;
        }
        auto line = input_.left(newline);
        input_.remove(0, newline + 1);
        if (line.endsWith('\r')) {
            line.chop(1);
        }
        emit lineReceived(QString::fromUtf8(line));
    }
    if (input_.size() > maximumLineBytes) {
        socket_.abort();
    }
}

GatewayConnection::~GatewayConnection() {
    // Stop callbacks before value members begin destruction.
    disconnect(&socket_, nullptr, this, nullptr);
    reconnectTimer_.stop();
    socket_.abort();
}