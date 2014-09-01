#include "networkhelper.h"


namespace JNRain {
int32_t NetworkHelper::getMaskFromPrefixLength(const unsigned int prefixLength) {
    unsigned int shift = 32 - prefixLength;
    return (int32_t)(((uint32_t)(-1) >> shift) << shift);
}

int NetworkHelper::getDefaultGateway(QHostAddress &address) {
    QString gwString = NetworkHelper::getDefaultGatewayAsString();

    if (gwString.isNull() || gwString.isEmpty()) {
        return -1;
    }

    address = *(new QHostAddress(gwString));
    return 0;
}

// 到平台实现的 delegate 方法
QString NetworkHelper::getDefaultGatewayAsString() {
    return pData().nhImpl->getDefaultGatewayAsString();
}

int NetworkHelper::queryRoutingTable() {
    return pData().nhImpl->queryRoutingTable();
}

int NetworkHelper::addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent) {
    return pData().nhImpl->addRoutingTableEntry(destination, prefixLength, next, isPersistent);
}

// 到大学实现的 delegate 方法
bool NetworkHelper::isIPInsideUniv(const QHostAddress &addr) {
    return pData().uhImpl->isIPInsideUniv(addr);
}
}
