//
// Created by Steve on 6/28/2020.
//

#include "msghdr-common.h"

MAIN() {
#define SERVER_IP "129.28.174.124"
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server = {
            .sin_family=AF_INET,
            .sin_port=htons(SERVER_PORT),
    };
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    char data[] = "Hello,there!";
    struct iovec iov = {
            .iov_base=data,
            .iov_len=sizeof(data),
    };
    struct msghdr msg = {
            .msg_name=&server,
            .msg_namelen=sizeof(server),
            .msg_iov=&iov,
            .msg_iovlen=1,
    };

    (void) sendmsg(sfd, &msg, 0);
    close(sfd);
    return 0;
}