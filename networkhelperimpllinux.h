#ifndef NETWORKHELPERIMPLLINUX_H
#define NETWORKHELPERIMPLLINUX_H

#include "networkhelperimplbase.h"


namespace JNRain {
    class NetworkHelperImplLinux : public NetworkHelperImplBase
    {
    public:
        virtual QString getDefaultGatewayAsString();
        virtual int queryRoutingTable();
        virtual int addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent);
    };
}

#endif // NETWORKHELPERIMPLLINUX_H
