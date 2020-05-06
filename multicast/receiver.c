//
// Created by Steve on 4/16/2020.
//

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"

int main() {
    int sock = -1;
    struct in_addr group_if = {};
    group_if.s_addr = inet_addr(GROUP_IP);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    ERROR_RETURN(sock == -1, -1, ,);

    int on = 1;
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEADDR, on, -1);
    SETSOCKOPT(sock, SOL_SOCKET, SO_REUSEPORT, on, -1);

    /* Bind ip & port */
    MAKE_SOCKADDR_IN(local, INADDR_ANY, htons(GROUP_PORT));
    ERROR_RETURN(bind(sock, SOCKADDR_EX(local)) < 0, -1, { close(sock); }, "bind: %m");

    /* Join group */
    sock_join_group(sock, group_if.s_addr, INADDR_ANY);

    /* Passive receiving */
    char recv_buf[1024];
    int recv_len;
    struct sockaddr_in from = {};
    socklen_t size = sizeof from;
    LOG("(MULTICAST)wait message");
    int recv_count = 0;
    while ((recv_len = recvfrom(sock, recv_buf, sizeof recv_buf, 0, (struct sockaddr *) &from, &size)) > 0) {
        ++recv_count;
        LOG("#%08d Receive: '%.*s' from %s:%d", recv_count, recv_len, recv_buf,
            inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    }
    close(sock);
    return 0;
}