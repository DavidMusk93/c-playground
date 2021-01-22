#include "callback.h"
#include "test.h"

using namespace sun::io;

MAIN() {
    using namespace std::placeholders;
    sun::io::TcpipServer tcpipServer(6666);
    Poll poll;
    poll.registerEntry(tcpipServer.transferOwnership(), EPOLLIN, std::bind(&sun::OnTcpipAccept, _1, &poll, &sun::Echo));
    poll.loop();
}