#include "networkhelperimplwindows.h"

#include <wbemidl.h>
#include <comdef.h>
#include <windows.h>
#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "Wbemuuid.lib")
#endif  // #ifdef _MSC_VER
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif  // #ifdef __GNUC__

#include <iostream>
#include <QtDebug>

#include "networkhelper.h"


namespace JNRain {
    QString NetworkHelperImplWindows::getDefaultGatewayAsString() {
        QString ret;

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

        return ret;
    }

    int NetworkHelperImplWindows::queryRoutingTable() {
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

            // Name
            _variant_t vaName;
            QString routeName;
            hr = pclsObj->Get(L"Name", 0, &vaName, NULL, NULL);
            if (hr == WBEM_S_NO_ERROR && vaName.vt != VT_NULL) {
                routeName = QString::fromUtf16(reinterpret_cast<const ushort *>(vaName.pbstrVal));
            }
            VariantClear(&vaName);

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
                    << "ROUTE: name" << routeName
                    << "caption" << routeCaption
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

        return 0;
    }

    int NetworkHelperImplWindows::addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent) {
        int ret = 0;

        HRESULT hres;
        IWbemLocator *pLoc = NULL;
        IWbemServices *pSvc = NULL;
        IEnumWbemClassObject *pEnumerator = NULL;
        IWbemClassObject *pEntryClass = NULL;
        IWbemClassObject *pInstance = NULL;
        _variant_t vaDestination;
        _variant_t vaMask;
        _variant_t vaNextHop;
        _variant_t vaMetric1;

        QString className(isPersistent ? "Win32_IP4PersistedRouteTable" : "Win32_IP4RouteTable");
        QString destAddrStr = destination.toString();
        QString netMaskStr = QHostAddress(NetworkHelper::getMaskFromPrefixLength(prefixLength)).toString();
        QString nextHopStr = next.toString();
        QString instanceQueryFormat("SELECT * FROM %1 WHERE Destination = \"%2\"");
        QString instanceQuery;

        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
        if (hres != S_OK && hres != RPC_E_TOO_LATE) {
            qDebug() << "CoInitializeSecurity failed:" << hres;
            ret = -1;
            goto out;
        }

        hres = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void **)&pLoc);
        if (FAILED(hres)) {
            qDebug() << "WbemAdministrativeLocator create failed:" << hres;
            ret = -2;
            goto out;
        }

        hres = pLoc->ConnectServer(L"root\\cimv2", NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
        if (FAILED(hres)) {
            qDebug() << "ConnectServer failed:" << hres;
            ret = -3;
            goto out;
        }

        // 检查是否已经有一条一样的路由规则
        // 先构造 WQL 查询语句
        instanceQuery = instanceQueryFormat.arg(className, destAddrStr);
        qDebug() << "Preflight query:" << instanceQuery;
        hres = pSvc->ExecQuery(
                    bstr_t(L"WQL"),
                    bstr_t(reinterpret_cast<const wchar_t *>(instanceQuery.utf16())),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pEnumerator
                    );
        if (FAILED(hres)) {
            qDebug() << "ExecQuery failed:" << hres;
            ret = -4;
            goto out;
        }

        while (pEnumerator) {
            ULONG uReturn;
            hres = pEnumerator->Next(WBEM_INFINITE, 1, &pInstance, &uReturn);
            if (!uReturn)
                break;

            // 找到第一个就行了
            if (pInstance) {
                qDebug() << "Existing routing table entry found";
                break;
            }
        }

        qDebug() << "pInstance =" << pInstance;
        if (!pInstance) {
            // 需要创建一个新的实例
            // 先拿到对应的 WMI 类...
            hres = pSvc->GetObject(
                        bstr_t(reinterpret_cast<const wchar_t *>(className.utf16())),
                        0,
                        NULL,
                        &pEntryClass,
                        NULL
                        );
            if (FAILED(hres)) {
                qDebug() << "Fetching of WMI class failed:" << hres;
                ret = -5;
                goto out;
            }

            // 然后创建实例
            hres = pEntryClass->SpawnInstance(0, &pInstance);
            if (FAILED(hres)) {
                qDebug() << "SpawnInstance failed:" << hres;
                ret = -6;
                goto out;
            }
        }
        qDebug() << "final pInstance =" << pInstance;

        // 填写信息
        // Destination
        vaDestination.vt = VT_BSTR;
        vaDestination.bstrVal = bstr_t(reinterpret_cast<const wchar_t *>(destAddrStr.utf16()));
        pInstance->Put(bstr_t(L"Destination"), 0, &vaDestination, 0);
        pInstance->Put(bstr_t(L"Name"), 0, &vaDestination, 0);
        pInstance->Put(bstr_t(L"Caption"), 0, &vaDestination, 0);
        pInstance->Put(bstr_t(L"Description"), 0, &vaDestination, 0);
        VariantClear(&vaDestination);

        // Mask
        vaMask.vt = VT_BSTR;
        vaMask.bstrVal = bstr_t(reinterpret_cast<const wchar_t *>(netMaskStr.utf16()));
        pInstance->Put(bstr_t(L"Mask"), 0, &vaMask, 0);
        VariantClear(&vaMask);

        // NextHop
        vaNextHop.vt = VT_BSTR;
        vaNextHop.bstrVal = bstr_t(reinterpret_cast<const wchar_t *>(nextHopStr.utf16()));
        pInstance->Put(bstr_t(L"NextHop"), 0, &vaNextHop, 0);
        VariantClear(&vaNextHop);

        // Metric1
        vaMetric1.vt = VT_I4;
        vaMetric1.intVal = 1;  // FIXME: 处理非永久路由情况
        pInstance->Put(bstr_t(L"Metric1"), 0, &vaMetric1, 0);
        VariantClear(&vaMetric1);

        // 写入 WMI
        // hres = pSvc->PutInstance(pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
        hres = pSvc->PutInstanceAsync(pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
        if (FAILED(hres)) {
            qDebug() << QString("PutInstance failed: 0x%1").arg(QString::number((uint32_t)hres, 16));
            ret = -7;
            goto out;
        }

out:
        if (pInstance)
            pInstance->Release();
        if (pEntryClass)
            pEntryClass->Release();
        if (pEnumerator)
            pEnumerator->Release();
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
        CoUninitialize();

        return ret;
    }
}
