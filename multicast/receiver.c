//
// Created by Steve on 4/16/2020.
//

#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

int main() {
    int sock = -1;
    struct in_addr groupAddr = {};
    groupAddr.s_addr = inet_addr(GROUP_IP);
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    int on = 1;
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEADDR, on, -1);
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEPORT, on, -1);

    /* Bind ip & port */
    MAKE_SOCKADDR_IN(localAddress, INADDR_ANY, htons(GROUP_PORT));
    if (bind(sock, (struct sockaddr *) &localAddress, sizeof localAddress) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    /* Join group */
    struct ip_mreq imr = {};
    imr.imr_multiaddr.s_addr = groupAddr.s_addr;
    imr.imr_interface.s_addr = INADDR_ANY;
    SETSOCKOPT(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, imr, -1);

    /* Passive receiving */
    char buf[1024];
    struct sockaddr_in from = {};
    socklen_t size = sizeof from;
    while (recvfrom(sock, buf, sizeof buf, 0, (struct sockaddr *) &from, &size) > 0) {
        printf("Receiving: %s from (%s,%d)\n", buf, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    }
    close(sock);
    return 0;
}