#pragma once
#include "../network/GatewayConnection.hpp"
#include "../protocol/GatewayProtocol.hpp"
#include "EventListModel.hpp"
#include <QObject>
#include <QVariantMap>

// Presents validated device state to QML. Device rules remain in firmware.
class RoomViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(bool online READ online NOTIFY changed)
    Q_PROPERTY(bool valid READ valid NOTIFY changed)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(bool pending READ pending NOTIFY changed)
    Q_PROPERTY(QVariantMap room READ room NOTIFY changed)
    Q_PROPERTY(QString result READ result NOTIFY changed)
    Q_PROPERTY(EventListModel* events READ events CONSTANT)
  public:
    explicit RoomViewModel(QObject* parent = nullptr);
    ~RoomViewModel() override;
    void start(const QString& host = "127.0.0.1", quint16 port = 5556);
    bool connected() const;
    bool online() const;
    bool valid() const;
    bool ready() const;
    bool pending() const;
    QVariantMap room() const;
    QString result() const;
    EventListModel* events();
    Q_INVOKABLE void setLight(bool enabled);
    Q_INVOKABLE void refresh();
  signals:
    void changed();
    void trafficObserved(const QString& line, bool outbound);

  private:
    GatewayConnection connection_;
    dashboard::GatewayStatus status_;
    QVariantMap room_;
    QString result_{"No command sent"};
    EventListModel events_;

    void handleConnection(bool connected);
    void handleLine(const QString& line);
    void applyState(const dashboard::RoomState& state);
    void applyResult(const dashboard::Message& message);
    void recordEvent(const QString& category, const QString& text);
};
