#include "../src/presentation/RoomViewModel.hpp"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTcpServer>
#include <QtTest>
#include <functional>
class UiTest : public QObject {
    Q_OBJECT
  private slots:
    void protocolValidation() {
        using dashboard::parseMessage;
        QVERIFY(parseMessage("ROOM STATE occupied=1 light_on=0 contact_open=0 alarm=none"));
        QVERIFY(!parseMessage("ROOM STATE occupied=x light_on=0 contact_open=0 alarm=none"));
        QVERIFY(!parseMessage("ROOM STATE occupied=0 light_on=0 contact_open=0 alarm=invalid"));
        QVERIFY(!parseMessage("GATEWAY STATUS online=1 valid=1 pending=0"));
        QVERIFY(!parseMessage("GATEWAY RESULT status=confirmed"));
        QVERIFY(!parseMessage("ROOM BOOT proto=2 node=smart_room"));
        QVERIFY(!parseMessage("ROOM BOOT proto=1 node=smart_room node=duplicate"));
        QCOMPARE(dashboard::lightCommand(true), QString("SET light_on 1"));
    }

    void controlAndReconnect() {
        QQuickStyle::setStyle("Basic");
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));
        RoomViewModel client;
        QQmlApplicationEngine engine;
        QSignalSpy warnings(&engine, &QQmlApplicationEngine::warnings);
        QSignalSpy traffic(&client, &RoomViewModel::trafficObserved);
        engine.rootContext()->setContextProperty("gateway", &client);
        engine.load(QUrl("qrc:/qml/Main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        window->resize(980, 680);
        auto toggle = window->findChild<QQuickItem*>("lightSwitch");
        QVERIFY(toggle);
        auto roomMap = window->findChild<QQuickItem*>("roomMap");
        QVERIFY(roomMap);
        QVERIFY(!roomMap->property("known").toBool());
        client.start("127.0.0.1", server.serverPort());
        QTRY_VERIFY(server.hasPendingConnections());
        auto peer = server.nextPendingConnection();
        QTRY_VERIFY(client.connected());
        QTRY_VERIFY(peer->bytesAvailable() > 0);
        QVERIFY(peer->readAll().contains("GET STATE"));
        QVERIFY(!client.ready());
        peer->write("ROOM STATE occupied=0 light_");
        QTest::qWait(30);
        QVERIFY(client.room().isEmpty());
        peer->write("on=0 contact_open=0 alarm=none\r\nGATEWAY STATUS online=1 valid=1 pending=0 "
                    "ready=1\n");
        QTRY_VERIFY(client.ready());
        QTRY_VERIFY(toggle->isEnabled());
        window->show();
        QTest::qWait(100);
        auto click = [&] {
            auto point =
                toggle->mapToScene(QPointF(toggle->width() / 2, toggle->height() / 2)).toPoint();
            QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
        };
        click();
        QTRY_VERIFY(client.pending());
        QTRY_VERIFY(peer->bytesAvailable() > 0);
        QVERIFY(peer->readAll().contains("SET light_on 1"));
        QCOMPARE(traffic.last().at(0).toString(), QString("SET light_on 1"));
        QVERIFY(traffic.last().at(1).toBool());
        QVERIFY(!client.room()["light_on"].toBool());
        QVERIFY(!toggle->isEnabled());
        peer->write("ROOM STATE occupied=0 light_on=1 contact_open=0 alarm=none\nGATEWAY RESULT "
                    "status=confirmed reason=state_verified\nGATEWAY STATUS online=1 valid=1 "
                    "pending=0 ready=1\n");
        QTRY_VERIFY(client.ready());
        QVERIFY(client.result().startsWith("Confirmed"));
        QTRY_VERIFY(toggle->property("checked").toBool());
        click();
        QTRY_VERIFY(client.pending());
        QTRY_VERIFY(peer->bytesAvailable() > 0);
        QVERIFY(peer->readAll().contains("SET light_on 0"));
        peer->write("GATEWAY RESULT status=rejected reason=device_rule\nGATEWAY STATUS online=1 "
                    "valid=1 pending=0 ready=1\n");
        QTRY_VERIFY(client.ready());
        QVERIFY(client.result().startsWith("Rejected"));
        QVERIFY(client.room()["light_on"].toBool());
        click();
        QTRY_VERIFY(client.pending());
        peer->disconnectFromHost();
        QTRY_VERIFY(!client.connected());
        QVERIFY(!client.ready());
        QTRY_VERIFY(!toggle->isEnabled());
        QTRY_VERIFY(!roomMap->property("live").toBool());
        QVERIFY(roomMap->property("lightOn").toBool()); // Cached, never presented as fresh.
        QVERIFY(client.result().startsWith("Unknown"));
        QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 4000);
        auto second = server.nextPendingConnection();
        QTRY_VERIFY(client.connected());
        QVERIFY(!client.ready());
        QTRY_VERIFY(second->bytesAvailable() > 0);
        QVERIFY(second->readAll().contains("GET STATE"));
        second->write("ROOM STATE occupied=1 light_on=1 contact_open=1 alarm=warning\nGATEWAY "
                      "STATUS online=1 valid=1 pending=0 ready=1\n");
        QTRY_VERIFY(client.ready());
        QVERIFY(client.room()["occupied"].toBool());
        QVERIFY(client.room()["contact_open"].toBool());
        QCOMPARE(client.room()["alarm"].toString(), QString("warning"));
        QTRY_VERIFY(roomMap->property("occupied").toBool());
        QTRY_VERIFY(roomMap->property("contactOpen").toBool());
        QTRY_COMPARE(roomMap->property("alarm").toString(), QString("warning"));
        QVERIFY(roomMap->property("live").toBool());

        // Exercise real QML navigation at the minimum supported window size.
        window->resize(640, 440);
        auto capture = [&](const QString& name) {
            QTest::qWait(100);
            const auto directory = qEnvironmentVariable("SMART_ROOM_TEST_SHOTS");
            if (!directory.isEmpty()) {
                QVERIFY(window->grabWindow().save(directory + "/" + name + ".png"));
            }
        };
        capture("overview-small");
        QCOMPARE(window->size(), QSize(640, 440));
        auto overview = window->findChild<QQuickItem*>("overviewPage");
        QVERIFY(overview);
        auto flickable = qvariant_cast<QObject*>(overview->property("contentItem"));
        QVERIFY(flickable);
        const auto wheelPoint = overview->mapToScene(QPointF(100, 100));
        QTest::wheelEvent(window, wheelPoint, QPoint(0, -120));
        QTRY_VERIFY(flickable->property("contentY").toReal() > 0);
        // Bring the real button into the viewport, then click at the small size.
        const auto localButton = toggle->mapToItem(overview, QPointF(0, 0));
        flickable->setProperty("contentY", flickable->property("contentY").toReal() + localButton.y() - 60);
        QTest::qWait(100);
        click();
        QTRY_VERIFY(client.pending());
        QTRY_VERIFY(second->bytesAvailable() > 0);
        QVERIFY(second->readAll().contains("SET light_on 0"));
        second->write("GATEWAY RESULT status=rejected reason=device_rule\nGATEWAY STATUS online=1 valid=1 pending=0 ready=1\n");
        QTRY_VERIFY(client.ready());
        capture("control-small");
        std::function<QQuickItem*(QQuickItem*, const QString&)> findVisualItem =
            [&](QQuickItem* parent, const QString& name) -> QQuickItem* {
            if (parent->objectName() == name)
                return parent;
            for (auto child : parent->childItems())
                if (auto found = findVisualItem(child, name))
                    return found;
            return nullptr;
        };
        for (int page : {1, 2, 0}) {
            auto nav = findVisualItem(window->contentItem(), QString("nav%1").arg(page));
            QVERIFY(nav);
            auto point = nav->mapToScene(QPointF(nav->width() / 2, nav->height() / 2)).toPoint();
            QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
            QTRY_COMPARE(window->property("page").toInt(), page);
            capture(QString("page-%1-small").arg(page));
        }
        auto events = window->findChild<QQuickItem*>("eventList");
        QVERIFY(events);
        QVERIFY(events->property("count").toInt() > 0);
        QCOMPARE(warnings.count(), 0);
    }
};
QTEST_MAIN(UiTest)
#include "ui_tests.moc"
