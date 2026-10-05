#include "presentation/RoomViewModel.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");
    RoomViewModel client;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gateway", &client);
    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;
    if (auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
        const auto available = window->screen()->availableGeometry();
        window->setPosition(available.center() - QPoint(window->width() / 2, window->height() / 2));
    }
    client.start();
    const auto args = app.arguments();
    int shot = args.indexOf("--screenshot");
    if (shot >= 0 && shot + 1 < args.size()) {
        auto path = args[shot + 1];
        QTimer::singleShot(3000, &app, [&engine, path] {
            if (auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first()))
                window->grabWindow().save(path);
        });
    }
    return app.exec();
}
