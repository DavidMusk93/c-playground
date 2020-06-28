//
// Created by Steve on 6/28/2020.
//

#include "msghdr-common.h"

MAIN_EX(argc, argv) {
    int sfd;
    struct sockaddr_in server = {}, client;
    char buf[MAXSIZE + 1];
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERVER_PORT);
    ERROR_RETURN(bind(sfd, SOCKADDR_EX(server)) == -1, 1, { close(sfd); }, "bind");

    struct msghdr msg;
    msg.msg_name = &client;
    msg.msg_namelen = sizeof(client);
    struct iovec iov = {
            .iov_base=buf,
            .iov_len=MAXSIZE,
    };
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    ssize_t len = recvmsg(sfd, &msg, 0);
    client = *(struct sockaddr_in *) msg.msg_name;
    char ip[16];
    inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
    short port = ntohs(client.sin_port);
    char *p = msg.msg_iov[0].iov_base;
    p[len] = 0;
    LOG("receive message from %s#%d: %s", ip, port, p);
    close(sfd);
    return 0;
}