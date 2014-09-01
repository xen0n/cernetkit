#ifndef NETWORKHELPERIMPLWINDOWS_H
#define NETWORKHELPERIMPLWINDOWS_H

#include "networkhelperimplbase.h"


namespace JNRain {
    class NetworkHelperImplWindows : public NetworkHelperImplBase
    {
    public:
        virtual QString getDefaultGatewayAsString();
        virtual int queryRoutingTable();
        virtual int addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent);
    };
}

#endif // NETWORKHELPERIMPLWINDOWS_H
