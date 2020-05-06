//
// Created by Steve on 4/16/2020.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>

#include "common.h"

in_addr_t G_SEND_ADDR = INADDR_ANY;
in_addr_t G_RECV_ADDR = INADDR_ANY;

int main(int argc, char *argv[]) {
    struct in_addr local_if = {};
    char buf[] = "Hi, there!";
    int buf_len = sizeof(buf) - 1;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int reuse = 1;
    char loop = 1;
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEADDR, reuse, -1);
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEPORT, reuse, -1);
    SETSOCKOPT(sock, IPPROTO_IP, IP_MULTICAST_LOOP, loop, -1);

    if (G_SEND_ADDR != INADDR_ANY) {
        local_if.s_addr = G_SEND_ADDR;
        SETSOCKOPT(sock, IPPROTO_IP, IP_MULTICAST_IF, local_if /* Sending interface */, -1);
    }

    /* Bind to group port (essential condition for receiving loopback message) */
    MAKE_SOCKADDR_IN(local, local_if.s_addr, htons(GROUP_PORT));
    ERROR_RETURN(bind(sock, SOCKADDR_EX(local)) < 0, -1, { close(sock); }, "bind: %m");

    MAKE_SOCKADDR_IN(group_addr, inet_addr(GROUP_IP), htons(GROUP_PORT));
    sock_join_group(sock, group_addr.sin_addr.s_addr, G_RECV_ADDR);
    /* (TTY) The thresholds enforce the conventions for multicast datagrams with initial values as follows:
     * 0      are restricted to the same host;
     * 1      are restricted to the same subnet;
     * 32     are restricted to the same site;
     * 64     are restricted to the same region;
     * 128    are restricted to the same continent;
     * 255    are unrestricted in scope. */
    //char tty = 0; /* time to live (default 1) */
    //SETSOCKOPT(sock, IPPROTO_IP, IP_MULTICAST_TTL, tty, -1);
    LOG("(MULTICAST)send message");
    int send_len = sendto(sock, buf, buf_len, 0, SOCKADDR_EX(group_addr));
    ERROR_RETURN(send_len != buf_len, -1, , "sendto() error: wrote %d bytes instead of %d: %m", send_len, buf_len);

    /* Try to read loopback message */
    struct pollfd pfd = {.fd=sock, .events=POLLIN};
    if (poll(&pfd, 1, 1000) == 1) {
        struct sockaddr_in from;
        socklen_t sl = sizeof from;
        char data[1024];
        int len = recvfrom(sock, data, sizeof data, 0, (struct sockaddr *) &from,
                           &sl /* could not retrieve &from if NULL */);
        LOG("(MULTICAST)loopback message: '%.*s' from %s:%d", len, data,
            inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    }
    LOG("eth0 ip: %s", inet_ntoa(my_ip(sock, "eth0")));
    close(sock);
    return 0;
}

