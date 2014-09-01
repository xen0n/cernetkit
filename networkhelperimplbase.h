#ifndef NETWORKHELPERIMPLBASE_H
#define NETWORKHELPERIMPLBASE_H

#include <stdint.h>
#include <QString>
#include <QHostAddress>


namespace JNRain {
    class NetworkHelperImplBase
    {
    public:
        NetworkHelperImplBase() {}
        virtual ~NetworkHelperImplBase() {}

        virtual QString getDefaultGatewayAsString() =0;
        virtual int queryRoutingTable() =0;
        virtual int addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent) =0;
    };
}

#endif // NETWORKHELPERIMPLBASE_H
