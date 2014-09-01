#ifndef NETWORKHELPERDATA_H
#define NETWORKHELPERDATA_H

#include "networkhelperimplbase.h"
#include "univhelperimplbase.h"

// 引入具体 NetworkHelper 实现
#ifdef Q_OS_WIN
#  include "networkhelperimplwindows.h"
#  define _NETWORKHELPERDATA_NH_IMPL_CHOSEN
#endif

#ifdef Q_OS_LINUX
#  include "networkhelperimpllinux.h"
#  define _NETWORKHELPERDATA_NH_IMPL_CHOSEN
#endif

#ifndef _NETWORKHELPERDATA_NH_IMPL_CHOSEN
#  error Current OS does not have a concrete NetworkHelperImpl!
#endif

// 引入具体 UnivHelper 实现
#ifdef UNIV_JIANGNAN
#  ifdef _NETWORKHELPERDATA_UH_IMPL_CHOSEN
#    error Multiple universities selected for compile!
#  else
#    include "univhelperimpljiangnan.h"
#    define _NETWORKHELPERDATA_UH_IMPL_CHOSEN
#  endif
#endif

#ifndef _NETWORKHELPERDATA_UH_IMPL_CHOSEN
#  error No university is selected for compile!
#endif


namespace JNRain {
class NetworkHelperData
{
public:
    JNRain::NetworkHelperImplBase *nhImpl;
    JNRain::UnivHelperImplBase *uhImpl;

    NetworkHelperData() {
#ifdef Q_OS_WIN
        nhImpl = new JNRain::NetworkHelperImplWindows();
#endif
#ifdef Q_OS_LINUX
        nhImpl = new JNRain::NetworkHelperImplLinux();
#endif

#ifdef UNIV_JIANGNAN
        uhImpl = new JNRain::UnivHelperImplJiangnan();
#endif
    }

    ~NetworkHelperData() {
        delete nhImpl;
        delete uhImpl;
    }
};
}

#endif // NETWORKHELPERDATA_H
