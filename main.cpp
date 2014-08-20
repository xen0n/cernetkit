#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "networkhelper.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();

    QString gw = JNRain::NetworkHelper::getDefaultGateway();
    ctx->setContextProperty("gw", gw);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
