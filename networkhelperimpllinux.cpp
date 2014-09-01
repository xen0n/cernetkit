#include "networkhelperimpllinux.h"

#define BUFSIZE 8192
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>

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


namespace JNRain {
    QString NetworkHelperImplLinux::getDefaultGatewayAsString() {
        QString ret;

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

        return ret;
    }

    int NetworkHelperImplLinux::queryRoutingTable() {
        // TODO
        return 0;
    }

    int NetworkHelperImplLinux::addRoutingTableEntry(const QHostAddress &destination, const int prefixLength, const QHostAddress &next, const bool isPersistent) {
        // TODO
        return 0;
    }
}
