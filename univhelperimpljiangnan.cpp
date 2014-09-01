#include "univhelperimpljiangnan.h"


namespace JNRain {
bool UnivHelperImplJiangnan::isIPInsideUniv(const QHostAddress &addr) {
    // TODO: 目前我们只关心宿舍区
    const QHostAddress dormSubnet("172.16.0.0");
    const int dormSubnetMask = 12;
    return addr.isInSubnet(dormSubnet, dormSubnetMask);
}
}
