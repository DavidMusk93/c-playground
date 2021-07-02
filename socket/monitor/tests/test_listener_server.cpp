#include "test.h"
#include "listener.h"

static void *gobj;

static void handleSigint(int sig) {
    if (gobj) {
        auto listener = reinterpret_cast<sun::Listener *>(gobj);
        listener->runOnLoop([listener] {
            listener->state = sun::Listener::State::QUIT;
        });
    }
}

MAIN_EX(argc, argv) {
    if (argc < 2) {
        LOGERROR("usage: %s port", argv[0]);
        return 1;
    }
    auto port = (short) atoi(argv[1]);
    using namespace sun;
    Listener listener(port);
    if (!listener.valid()) {
        return 2;
    }
    gobj = &listener;
    INSTALLSIGINTHANDLER(handleSigint);
    listener.run();
    return 0;
}