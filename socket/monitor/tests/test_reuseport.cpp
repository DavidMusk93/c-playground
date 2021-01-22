#include "callback.h"
#include "test.h"

DECLAREPOLLSIGNALHANDLER(g_handler, handle_signal);

MAIN() {
    INSTALLSIGINTHANDLER(&handle_signal);
    using namespace sun;
    using namespace std::placeholders;
    io::TcpipServer tcpipServer(6666, true);
    tcpipServer.enableSharePort();
    io::Poll poll;
    g_handler = &poll;
    Callback echo_hook = [](int fd) {
        Echo(fd);
        LOGINFO("Sleep 5000 milliseconds");
        utility::Sleep(5000);
    };
    poll.registerEntry(tcpipServer.initialize().transferOwnership(), EPOLLIN,
                       std::bind(&OnTcpipAccept, _1, &poll, echo_hook));
    poll.loop();
}