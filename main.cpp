#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtDebug>

#include "networkhelper.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();

    QHostAddress gw;
    int ret = JNRain::NetworkHelper::getDefaultGateway(gw);
    if (!ret) {
        ctx->setContextProperty("gw", gw.toString());
        ctx->setContextProperty("gwInsideUniv", JNRain::NetworkHelper::isIPInsideUniv(gw));
    } else {
        ctx->setContextProperty("gw", "failed to get gw");
    }

    // test
    JNRain::NetworkHelper::queryRoutingTable();
    qDebug() << "netmask for /12 =" << QHostAddress(JNRain::NetworkHelper::getMaskFromPrefixLength(12)).toString();

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
