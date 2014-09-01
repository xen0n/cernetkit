#include "networkhelperimplwindows.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "iphlpapi.lib")
#endif

#include <QtDebug>

#include "networkhelper.h"


namespace JNRain {
    QString NetworkHelperImplWindows::getDefaultGatewayAsString() {
        QString ret;

        MIB_IPFORWARD_TABLE2 *table = NULL;
        NETIO_STATUS status;

        status = GetIpForwardTable2(AF_INET, &table);
        if (!NETIO_SUCCESS(status)) {
            qDebug() << "GetIpForwardTable2 failed:" << status;
            return 1;
        }

        for (ULONG i = 0; i < table->NumEntries; i++) {
            MIB_IPFORWARD_ROW2 *row = table->Table + i;
            IP_ADDRESS_PREFIX *prefix = &row->DestinationPrefix;
            SOCKADDR_INET *destAddr = &prefix->Prefix;

            if (destAddr->Ipv4.sin_addr.S_un.S_addr == 0) {
                ret = QHostAddress(ntohl(row->NextHop.Ipv4.sin_addr.S_un.S_addr)).toString();
                break;
            }
        }

        FreeMibTable(table);

        return ret;
    }

    int NetworkHelperImplWindows::queryRoutingTable() {
        MIB_IPFORWARD_TABLE2 *table = NULL;
        NETIO_STATUS status;

        status = GetIpForwardTable2(AF_INET, &table);
        if (!NETIO_SUCCESS(status)) {
            qDebug() << "GetIpForwardTable2 failed:" << status;
            return 1;
        }

        for (ULONG i = 0; i < table->NumEntries; i++) {
            MIB_IPFORWARD_ROW2 *row = table->Table + i;
            IP_ADDRESS_PREFIX *prefix = &row->DestinationPrefix;
            QHostAddress destAddr(ntohl(prefix->Prefix.Ipv4.sin_addr.S_un.S_addr));
            QHostAddress nextHop(ntohl(row->NextHop.Ipv4.sin_addr.S_un.S_addr));

#ifdef _DEBUG
            const QString debugLineFormat("%1/%2 via %3 metric %4 dev %5  # %6proto=%7, origin=%8");

            QString debugLine = debugLineFormat.arg(
                    destAddr.toString(),
                    QString::number(prefix->PrefixLength),
                    nextHop.toString(),
                    QString::number(row->Metric),
                    (row->Loopback ? "lo" : QString::number(row->InterfaceIndex)),
                    (row->Immortal ? "immortal, " : ""),
                    QString::number(row->Protocol),
                    QString::number(row->Origin)
                    );

            qDebug() << debugLine;
#endif
        }

        FreeMibTable(table);

        return 0;
    }

    int NetworkHelperImplWindows::addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent) {
        // TODO
        return 0;
    }
}
