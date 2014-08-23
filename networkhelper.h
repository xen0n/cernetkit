#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <QString>
#include <QHostAddress>


namespace JNRain {
    class NetworkHelper
    {
    public:
        static QString getDefaultGatewayAsString();
        static int getDefaultGateway(QHostAddress &addr);
        static bool isIPInsideUniv(const QHostAddress &addr);
        static int queryRoutingTable();
    };
}

#endif // NETWORKHELPER_H
