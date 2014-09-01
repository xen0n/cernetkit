#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <stdint.h>
#include <QString>
#include <QHostAddress>

#include "networkhelperdata.h"


namespace JNRain {
    class NetworkHelper
    {
        friend class JNRain::NetworkHelperData;

    public:
        static int32_t getMaskFromPrefixLength(const unsigned int prefixLength);
        static int getDefaultGateway(QHostAddress &addr);

        static QString getDefaultGatewayAsString();
        static int queryRoutingTable();
        static int addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent);

        static bool isIPInsideUniv(const QHostAddress &addr);

    private:
        static JNRain::NetworkHelperData &pData() {
            static JNRain::NetworkHelperData data;

            return data;
        }
    };
}

#endif // NETWORKHELPER_H
