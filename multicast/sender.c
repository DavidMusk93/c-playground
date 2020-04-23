//
// Created by Steve on 4/16/2020.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "common.h"

in_addr_t g_sending_interface_addr = INADDR_ANY;
in_addr_t g_receiving_interface_addr = INADDR_ANY;

int main(int argc, char *argv[]) {
    struct in_addr localInterface = {};
    char buffer[] = "Hi, there!";
    int bufferLength = sizeof(buffer);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int reuse = 1;
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEADDR, reuse, -1);
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEPORT, reuse, -1);

    /* Receiver address */
    MAKE_SOCKADDR_IN(groupAddress, inet_addr(GROUP_IP), htons(GROUP_PORT));

    /* Enable loopback */
    char loop = 1;
    SETSOCKOPT(sock, IPPROTO_IP, IP_MULTICAST_LOOP, loop, -1);

    /* Set local interface (multicast capable) for outbound multicast datagram */
    // g_sending_interface_addr = inet_addr("192.168.1.101");
    if (g_sending_interface_addr != INADDR_ANY) {
        localInterface.s_addr = g_sending_interface_addr;
        SETSOCKOPT(sock, IPPROTO_IP, IP_MULTICAST_IF, localInterface /* Sending interface */, -1);
    }

    /* Bind to group port (essential condition for receiving loopback message) */
    MAKE_SOCKADDR_IN(local, localInterface.s_addr, htons(GROUP_PORT));
    if (bind(sock, SOCKADDR_EXT(local)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    /* Join group */
    struct ip_mreq imr = {};
    imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
    // imr.imr_interface.s_addr = localInterface.s_addr; /* Receiving interface */
    imr.imr_interface.s_addr = g_receiving_interface_addr;
    SETSOCKOPT(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, imr, -1);

    /* Send message */
    puts("Sending datagram message...");
    sendto(sock, buffer, bufferLength, 0, SOCKADDR_EXT(groupAddress));

    /* Try to read loopback message */
    struct pollfd pfd = {.fd=sock, .events=POLLIN};
    if (poll(&pfd, 1, 1000) == 1) {
        char readBuffer[1024];
        int bytesRead = read(sock, readBuffer, sizeof readBuffer);
        printf("Loopback message: %*s\n", bytesRead, readBuffer);
    }
    close(sock);
    return 0;
}

