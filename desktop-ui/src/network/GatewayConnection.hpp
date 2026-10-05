#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

// Transports bounded lines. It deliberately knows nothing about room state.
class GatewayConnection : public QObject {
    Q_OBJECT
  public:
    explicit GatewayConnection(QObject* parent = nullptr);
    ~GatewayConnection() override;
    void start(const QString& host, quint16 port);
    bool isConnected() const;
    void sendLine(const QString& line);
  signals:
    void connectionChanged(bool connected);
    void lineReceived(const QString& line);

  private:
    static constexpr int reconnectIntervalMs = 2000;
    static constexpr qsizetype maximumLineBytes = 1024;
    QTcpSocket socket_;
    QTimer reconnectTimer_;
    QByteArray input_;
    QString host_;
    quint16 port_{5556};
    void connectNow();
    void receiveBytes();
    void handleDisconnect();
};
