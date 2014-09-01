#ifndef UNIVHELPERIMPLBASE_H
#define UNIVHELPERIMPLBASE_H

#include <QHostAddress>


namespace JNRain {
    class UnivHelperImplBase
    {
    public:
        UnivHelperImplBase() {}
        virtual ~UnivHelperImplBase() {}

        virtual bool isIPInsideUniv(const QHostAddress &addr) =0;
    };
}

#endif // UNIVHELPERIMPLBASE_H
