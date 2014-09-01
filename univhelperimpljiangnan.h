#ifndef UNIVHELPERIMPLJIANGNAN_H
#define UNIVHELPERIMPLJIANGNAN_H

#include "univhelperimplbase.h"


namespace JNRain {
    class UnivHelperImplJiangnan : public UnivHelperImplBase
    {
    public:
        virtual bool isIPInsideUniv(const QHostAddress &addr);
    };
}

#endif // UNIVHELPERIMPLJIANGNAN_H
