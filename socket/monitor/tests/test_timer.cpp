#include "test.h"
#include "timer.h"
#include "sock.h"

DECLAREPOLLSIGNALHANDLER(g_handler, handle_signal);

MAIN() {
    INSTALLSIGINTHANDLER(&handle_signal);
    sun::io::Poll poll;
    g_handler = &poll;
    sun::Timer::Config config(1);
    sun::Timer timer(config);
    poll.registerEntry(timer.transferOwnership(), EPOLLIN, &sun::Timer::OnTimeout);
    poll.loop();
}