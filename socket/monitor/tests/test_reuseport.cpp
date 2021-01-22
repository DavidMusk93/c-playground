#include "callback.h"
#include "test.h"

#define DEFAULTDELAY 10000

DECLAREPOLLSIGNALHANDLER(g_handler, handle_signal);

MAIN_EX(argc, argv) {
    INSTALLSIGINTHANDLER(&handle_signal);
    using namespace sun;
    using namespace std::placeholders;
    io::TcpipServer tcpipServer(6666, true);
    tcpipServer.enableSharePort();
    io::Poll poll;
    g_handler = &poll;
    int ms = argc > 1 ? ({
        int val = atoi(argv[1]);
        val ? (val < DEFAULTDELAY ? DEFAULTDELAY : val) : 0/*disable sleep*/;
    }) : DEFAULTDELAY;
    Callback echo_hook = [ms](int fd) {
        Echo(fd);
        if (ms) {
            util::TimeThis timeThis("block echo");
            util::Sleep(ms);
        }
    };
    poll.registerEntry(tcpipServer.initialize().transferOwnership(), EPOLLIN,
                       std::bind(&OnTcpipAccept, _1, &poll, echo_hook));
    poll.loop();
}