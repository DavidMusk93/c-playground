#include "callback.h"

#include <unistd.h>

namespace sun {
    void Echo(int fd) {
        char buf[1024];
        // @TODO One should read as much as possible
        int nr = read(fd, buf, sizeof(buf));
        if (nr > 0) {
            buf[nr] = 0;
            FUNCLOG("#%d new Message %d,'%s'", fd, nr, buf);
            write(fd, buf, nr);
        }
    }

    void OnTcpipAccept(int fd, void *handler, const Callback &callback) {
        struct sockaddr_in peer{};
        socklen_t len = sizeof(peer);
        int sock{-1};
        ERRRET((sock = accept4(fd, SOCKADDRPTR(&peer), &len, 0)) == -1, , , 1, "accept4");
        FUNCLOG("new Connection " SOCKADDR_FMT, SOCKADDR_OF(peer));
        reinterpret_cast<io::Poll *>(handler)->registerEntry(sock, EPOLLRDHUP | EPOLLIN, callback);
    }
}