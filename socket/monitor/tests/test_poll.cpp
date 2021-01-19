#include "sock.h"
#include "test.h"

using namespace sun::io;

static void Echo(int fd) {
    char buf[1024];
    int nr = read(fd, buf, sizeof(buf));
    if (nr > 0) {
        buf[nr] = 0;
        LOGINFO("new Message '%s'", buf);
        write(fd, buf, nr);
    }
}

static void DoAccept(int fd, Poll *handler) {
    struct sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    int sock{-1};
    ERRRET((sock = accept4(fd, SOCKADDRPTR(&peer), &len, 0)) == -1, , , 1, "accept4");
    LOGINFO("new Connection " SOCKADDR_FMT, SOCKADDR_OF(peer));
    handler->registerEntry(sock, EPOLLRDHUP | EPOLLIN, &Echo, {});
}

MAIN() {
    using namespace std::placeholders;
    sun::io::TcpipServer tcpipServer(6666);
    Poll poll;
    poll.registerEntry(tcpipServer.transferOwnership(), EPOLLIN, std::bind(&DoAccept, _1, &poll), {});
    poll.loop();
}