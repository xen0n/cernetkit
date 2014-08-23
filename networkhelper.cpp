#include "networkhelper.h"

#ifdef Q_OS_WIN
#  include <wbemidl.h>
#  include <comdef.h>
#  include <windows.h>
#  ifdef _MSC_VER
#    pragma comment(lib, "ws2_32.lib")
#    pragma comment(lib, "Wbemuuid.lib")
#  endif  // #ifdef _MSC_VER
#  ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wwrite-strings"
#  endif  // #ifdef __GNUC__
#include <iostream>
#include <QtDebug>
#else  // #ifdef Q_OS_WIN
#  define BUFSIZE 8192
#  include <arpa/inet.h>
#  include <net/if.h>
#  include <sys/socket.h>
#  include <linux/rtnetlink.h>
#  include <unistd.h>
#  include <stdio.h>

struct route_info {
    in_addr dstAddr;
    in_addr srcAddr;
    in_addr gateWay;
    char ifName[IF_NAMESIZE];
};


static int readNlSock(int sockFd, char *bufPtr, unsigned int seqNum, int pId) {
    struct nlmsghdr *nlHdr;
    int readLen = 0, msgLen = 0;
    do {
        if ((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
            perror("SOCK READ: ");
            return -1;
        }
        nlHdr = (struct nlmsghdr *)bufPtr;
        if ((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
            perror("Error in received packet");
            return -1;
        }
        if (nlHdr->nlmsg_type == NLMSG_DONE)
            break;

        bufPtr += readLen;
        msgLen += readLen;

        if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
            break;

    } while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != (unsigned)pId));
    return msgLen;
}

static void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo) {
    struct rtmsg *rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
        return;

    struct rtattr *rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    int rtLen = RTM_PAYLOAD(nlHdr);
    for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
        switch (rtAttr->rta_type) {
            case RTA_OIF:
                if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
                break;
            case RTA_GATEWAY:
                rtInfo->gateWay = *(in_addr *)RTA_DATA(rtAttr);
                break;
            case RTA_PREFSRC:
                rtInfo->srcAddr = *(in_addr *)RTA_DATA(rtAttr);
                break;
            case RTA_DST:
                rtInfo->dstAddr = *(in_addr *)RTA_DATA(rtAttr);
                break;
        }
    }
}
#endif  // #ifdef Q_OS_WIN


namespace JNRain {
    QString NetworkHelper::getDefaultGatewayAsString() {
        QString ret;
#ifdef Q_OS_WIN
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        HRESULT securityInitRet = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
        if (securityInitRet != S_OK && securityInitRet != RPC_E_TOO_LATE)
            return ret;

        IWbemLocator *pLoc = NULL;
        if (CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void **)&pLoc) != S_OK)
            return ret;

        IWbemServices *pSvc = NULL;
        if (pLoc->ConnectServer(L"root\\cimv2", NULL, NULL, NULL, 0, NULL, NULL, &pSvc) != S_OK)
            return ret;

        IEnumWbemClassObject *pEnumerator = NULL;
        HRESULT hr = pSvc->ExecQuery(L"WQL", L"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = 'TRUE'", WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return ret;
        }

        IWbemClassObject *pclsObj = NULL;
        while (pEnumerator && ret.isEmpty()) {
            _variant_t vaDefaultIPGateway;
            ULONG uReturn;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (!uReturn)
                break;

            hr = pclsObj->Get(L"DefaultIPGateway", 0, &vaDefaultIPGateway, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaDefaultIPGateway.vt != VT_NULL) {
                LONG lLow = 0;
                LONG lUp = 0;
                ::SafeArrayGetLBound(vaDefaultIPGateway.parray, 1, &lLow);
                ::SafeArrayGetUBound(vaDefaultIPGateway.parray, 1, &lUp);
                for (LONG i = lLow; i <= lUp; i++) {
                    BSTR bsDefaultIPGateway;
                    if (::SafeArrayGetElement(vaDefaultIPGateway.parray, &i, &bsDefaultIPGateway) == S_OK) {
                        ret = QString::fromUtf16(reinterpret_cast<const ushort *>(bsDefaultIPGateway));
                        ::SysFreeString(bsDefaultIPGateway);
                        break;
                    }
                }
            }
            VariantClear(&vaDefaultIPGateway);
        }

        if (pclsObj)
            pclsObj->Release();
        if (pEnumerator)
            pEnumerator->Release();
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
        CoUninitialize();
#else  // #ifdef Q_OS_WIN
        int sock, msgSeq = 0;
        if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
            perror("Socket Creation: ");
            return ret;
        }

        char msgBuf[BUFSIZE];
        memset(msgBuf, 0, BUFSIZE);
        struct nlmsghdr *nlMsg = (struct nlmsghdr *)msgBuf;

        nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
        nlMsg->nlmsg_type = RTM_GETROUTE;

        nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
        nlMsg->nlmsg_seq = msgSeq++;
        nlMsg->nlmsg_pid = getpid();

        if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
            return ret;

        int len;
        if ((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0)
            return ret;

        struct route_info *rtInfo = (struct route_info *)malloc(sizeof(struct route_info));

        for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
            memset(rtInfo, 0, sizeof(struct route_info));
            parseRoutes(nlMsg, rtInfo);

            if (strstr((char *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0") && !strstr((char *)inet_ntoa(rtInfo->gateWay), "0.0.0.0")) {
                char buf[64];
                inet_ntop(AF_INET, &rtInfo->gateWay, buf, sizeof(buf));
                ret = QString(buf);
                break;
            }
        }

        free(rtInfo);
        close(sock);
#endif  // #ifdef Q_OS_WIN
        return ret;
    }

    int NetworkHelper::getDefaultGateway(QHostAddress &address) {
        QString gwString = NetworkHelper::getDefaultGatewayAsString();

        if (gwString.isNull() || gwString.isEmpty()) {
            return -1;
        }

        address = *(new QHostAddress(gwString));
        return 0;
    }

    bool NetworkHelper::isIPInsideUniv(const QHostAddress &addr) {
        // TODO: 目前我们只关心宿舍区
        const QHostAddress dormSubnet("172.16.0.0");
        const int dormSubnetMask = 12;

        return addr.isInSubnet(dormSubnet, dormSubnetMask);
    }

    int NetworkHelper::queryRoutingTable() {
#ifdef Q_OS_WIN
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        HRESULT securityInitRet = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
        if (securityInitRet != S_OK && securityInitRet != RPC_E_TOO_LATE)
            return -1;

        IWbemLocator *pLoc = NULL;
        if (CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void **)&pLoc) != S_OK)
            return -2;

        IWbemServices *pSvc = NULL;
        if (pLoc->ConnectServer(L"root\\cimv2", NULL, NULL, NULL, 0, NULL, NULL, &pSvc) != S_OK)
            return -3;

        IEnumWbemClassObject *pEnumerator = NULL;
        HRESULT hr = pSvc->ExecQuery(L"WQL", L"SELECT * FROM Win32_IP4RouteTable", WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return -4;
        }

        IWbemClassObject *pclsObj = NULL;
        while (pEnumerator) {
            ULONG uReturn;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (!uReturn)
                break;

            // Caption
            _variant_t vaCaption;
            QString routeCaption;
            hr = pclsObj->Get(L"Caption", 0, &vaCaption, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaCaption.vt != VT_NULL) {
                routeCaption = QString::fromUtf16(reinterpret_cast<const ushort *>(vaCaption.pbstrVal));
            }
            VariantClear(&vaCaption);

            // Description
            _variant_t vaDescription;
            QString routeDesc;
            hr = pclsObj->Get(L"Description", 0, &vaDescription, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaDescription.vt != VT_NULL) {
                routeDesc = QString::fromUtf16(reinterpret_cast<const ushort *>(vaDescription.pbstrVal));
            }
            VariantClear(&vaCaption);

            // Destination
            _variant_t vaDestination;
            QString routeDest;
            hr = pclsObj->Get(L"Destination", 0, &vaDestination, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaDestination.vt != VT_NULL) {
                routeDest = QString::fromUtf16(reinterpret_cast<const ushort *>(vaDestination.pbstrVal));
            }
            VariantClear(&vaDestination);

            // NextHop
            _variant_t vaNextHop;
            QString routeNext;
            hr = pclsObj->Get(L"NextHop", 0, &vaNextHop, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaNextHop.vt != VT_NULL) {
                routeNext = QString::fromUtf16(reinterpret_cast<const ushort *>(vaNextHop.pbstrVal));
            }
            VariantClear(&vaNextHop);

            // Mask
            _variant_t vaMask;
            QString routeMask;
            hr = pclsObj->Get(L"Mask", 0, &vaMask, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaMask.vt != VT_NULL) {
                routeMask = QString::fromUtf16(reinterpret_cast<const ushort *>(vaMask.pbstrVal));
            }
            VariantClear(&vaMask);

            qDebug()
                    << "ROUTE: caption" << routeCaption
                    << "desc" << routeDesc
                    << "dest" << routeDest
                    << "mask" << routeMask
                    << "next" << routeNext
                    ;
        }

        if (pclsObj)
            pclsObj->Release();
        if (pEnumerator)
            pEnumerator->Release();
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
        CoUninitialize();
#else  // #ifdef Q_OS_WIN
        // TODO
#endif  // #ifdef Q_OS_WIN
        return 0;
    }

    int32_t NetworkHelper::getMaskFromPrefixLength(const unsigned int prefixLength) {
        unsigned int shift = 32 - prefixLength;
        return (int32_t)(((uint32_t)(-1) >> shift) << shift);
    }
}
