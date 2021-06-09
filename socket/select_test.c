#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "macro.h"

int new_server(short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {.sin_family=AF_INET, .sin_port=htons(port),};
    if (fd == -1) {
        return -1;
    }
    int REUSE = 1;
    int rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &REUSE, sizeof(REUSE));
    if (rc == -1) {
        goto err;
    }
    rc = bind(fd, (struct sockaddr *) &addr, sizeof addr);
    if (rc == -1) {
        goto err;
    }
    rc = listen(fd, 1);
    if (rc == -1) {
        goto err;
    }
    return fd;
err:
    close(fd);
    return -1;
}

int on_recv(int fd) {
    static char buf[64];
    int nr = (int) read(fd, buf, sizeof buf);
    if (nr <= 0) {
        if (errno != 0) {
            LOG_ERROR("@%s error %d,%d,%s", __func__, nr, errno, ERROR_S);
        }
        return -1;
    }
    LOG_INFO("@%s '%.*s'", __func__, nr, buf);
    return (int) write(fd, buf, nr);
}

MAIN_EX(argc, argv) {
    if (argc < 2) {
        LOG_ERROR("server port is missing");
        return -1;
    }
    int port = atoi(argv[1]);
    if (port < 100) {
        port = 8888;
    }
    int sfd, cfd;
    int rc;
    sfd = new_server(port);
    if (sfd == -1) {
        LOG_ERROR("initialize server error: %s", ERROR_S);
        return -1;
    }
    struct sockaddr_in peer = {};
    socklen_t l = sizeof peer;
    fd_set fds = {};
again:
    errno = 0;
    cfd = accept(sfd, (struct sockaddr *) &peer, &l);
    if (cfd == -1) {
        perror("accept");
        return -1;
    }
    LOG("new connection from %s:%d", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
    FD_SET(cfd, &fds);
    for (;;) {
        rc = select(cfd + 1, &fds, 0, 0, 0);
        if (rc < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            break;
        }
        if (on_recv(cfd) == -1) {
            LOG("client quit");
            goto again;
        }
    }
    return 0;
}