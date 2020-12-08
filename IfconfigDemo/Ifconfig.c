#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/hdreg.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include<string.h>
#include <net/if.h>
#include<errno.h>
#define IF_NAMESIZE	16
struct interface {
    int index;
    int flags; /* IFF_UP etc. */
    long speed; /* Mbps */
    int duplex; /* DUPLEX_FULL-全双工, DUPLEX_HALF-半双工 */
    int mtu;
    char name[IF_NAMESIZE + 1]; /*name*/
    char ipaddr[64]; /*ip address*/
    char hwaddr[64]; /*MAC address*/
    char netmask[64]; /*network mask*/
};

int GetInterfaceCommon(struct ifreq * const ifr,
        struct interface * const info) {
    struct ethtool_cmd cmd;
    int result;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {

        return 0;
    }

    /* Interface flags. */
    if (ioctl(sockfd, SIOCGIFFLAGS, ifr) == -1)
        info->flags = 0;
    else
        info->flags = ifr->ifr_flags;

    /* Get mtu. */
    if (ioctl(sockfd, SIOCGIFMTU, ifr) == -1)
        info->mtu = 0;
    else
        info->mtu = ifr->ifr_mtu;

    /* Get ip address. */
    if (ioctl(sockfd, SIOCGIFADDR, ifr) == -1) {
        strncpy(info->ipaddr, "0.0.0.0", 64);
    } else {
        struct sockaddr *address = &(ifr->ifr_addr);
        struct sockaddr_in *address_in = (struct sockaddr_in *) address;
        strncpy(info->ipaddr, inet_ntoa(address_in->sin_addr), 64);
    }

    /* Get Net Mask. */
    if (ioctl(sockfd, SIOCGIFNETMASK, ifr) == -1) {
        strncpy(info->netmask, "255.255.255.255", 64);
    } else {
        struct sockaddr *address = &(ifr->ifr_netmask);
        struct sockaddr_in *address_in = (struct sockaddr_in *) address;
        strncpy(info->netmask, inet_ntoa(address_in->sin_addr), 64);
    }

    /* Get Hardware address. */
    if (ioctl(sockfd, SIOCGIFHWADDR, ifr) == -1) {
        strncpy(info->hwaddr, "00:00:00:00:00:00", 64);
    } else {
        unsigned char *mac = (unsigned char *) ifr->ifr_hwaddr.sa_data;
        snprintf(info->hwaddr, 64, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    ifr->ifr_data = (char *) &cmd;
    cmd.cmd = ETHTOOL_GSET; /* "Get settings" */
    if (ioctl(sockfd, SIOCETHTOOL, ifr) == -1) {
        /* Unknown */
        info->speed = -1L;
        info->duplex = -1;
    } else {
        info->speed = ethtool_cmd_speed(&cmd);
        info->duplex = cmd.duplex;
    }

    do {
        result = close(sockfd);
    } while (result == -1 && errno == EINTR);

    return 0;
}

int GetNicCount() {
    struct ifconf ifc;
    struct ifreq ifs[16];
    int sockfd;
    int result;
    int count = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error: create socket error (%s).", strerror(errno));
        return 0;
    }

    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;

    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        do {
            result = close(sockfd);
        } while (result == -1 && errno == EINTR);

        fprintf(stderr, "Error: ioctl error (%s).", strerror(errno));
        return 0;
    }

    count = ifc.ifc_len / sizeof(struct ifreq);

    do {
        result = close(sockfd);
    } while (result == -1 && errno == EINTR);
    return count;
}

int GetNicInfoByIndex(const int index, struct interface * const info) {
    int sockfd, result;
    struct ifreq ifr;

    /*if (index < 1 || !info) {
        fprintf(stderr, "Error: params invalid!.");
        return -1;
    }*/

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error: create socket error (%s).", strerror(errno));
        return -2;
    }

    ifr.ifr_ifindex = index;
    if (ioctl(sockfd, SIOCGIFNAME, &ifr) == -1) {
        do {
            result = close(sockfd);
        } while (result == -1 && errno == EINTR);

        fprintf(stderr, "Error: ioctl error (%s).", strerror(errno));
        return -3;
    }

    info->index = index;
    strncpy(info->name, ifr.ifr_name, IF_NAMESIZE);


    info->name[IF_NAMESIZE] = '\0';

    do {
        result = close(sockfd);
    } while (result == -1 && errno == EINTR);

    return GetInterfaceCommon(&ifr, info);
}


void GetLocalCofig()
{
    int num = GetNicCount();
    int i = 0;
    while(1)
    {
        char szGateWay[30]={0};
        struct interface inter;
        memset(&inter, 0, sizeof(struct interface));
        GetNicInfoByIndex(i, &inter);
        i++;
        printf("*******************************************\n");
        printf("inter name :%s \n",inter.name );
        printf("inter m_strRealMac :%s \n",inter.hwaddr );
        printf("inter m_strIp :%s \n",inter.ipaddr );
        printf("*******************************************\n");
        //printf("inter m_strMark :%s \n",inter.m_strMark );
        //printf("inter m_strGetway :%s \n",inter.m_strGetway );
        if(i>5)
        {
            break;
        }
    }
}

int main(void)
{
    GetLocalCofig();
    return 0;
}

