#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <stdint.h>
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
        static int32_t getMaskFromPrefixLength(const unsigned int prefixLength);
        static int addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent);
    };
}

#endif // NETWORKHELPER_H
