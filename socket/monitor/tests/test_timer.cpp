#include "test.h"
#include "timer.h"
#include "sock.h"

static void *g_handler;

static void SigHandler(int sig) {
    if (g_handler) {
        reinterpret_cast<sun::io::Poll *>(g_handler)->quit();
    }
}

MAIN() {
    INSTALLSIGINTHANDLER(&SigHandler);
    sun::io::Poll poll;
    g_handler = &poll;
    sun::Timer::Config config(1, 0);
    sun::Timer timer(config);
    poll.registerEntry(timer.transferOwnership(), EPOLLIN, &sun::Timer::OnTimeout, {});
    poll.loop();
}