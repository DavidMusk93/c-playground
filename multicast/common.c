//
// Created by Steve on 4/29/2020.
//

#include <netdb.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "common.h"

bool sock_join_group(int sock, in_addr_t group, in_addr_t recv_if) {
    struct ip_mreq imr = {
            .imr_multiaddr.s_addr = group,
            .imr_interface.s_addr = recv_if,
    };
    SETSOCKOPT(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, imr, false);
    return true;
}

bool sock_make_nonblock(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
}

struct in_addr my_ip(int sock, const char *if_name) {
    struct ifreq ifr = {};
    ifr.ifr_addr.sa_family = AF_INET;
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", if_name);
    ioctl(sock, SIOCGIFADDR, &ifr);
    return RETRIEVE_ADDR(&ifr.ifr_addr);
}