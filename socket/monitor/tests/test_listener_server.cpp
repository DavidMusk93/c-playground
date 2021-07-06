#include "test.h"
#include "listener.h"

static void *gobj;

static void handlesig(int sig) {
    if (gobj) {
        auto listener = reinterpret_cast<sun::Listener *>(gobj);
        listener->runOnLoop([listener] {
            listener->setstate(stateful::State::TERMINATED);
        });
    }
}

MAIN_EX(argc, argv) {
    SETNOBUF();
    if (argc < 2) {
        LOGERROR("usage: %s port", argv[0]);
        return 1;
    }
    auto port = (short) atoi(argv[1]);
    using namespace sun;
    Listener listener(port);
    if (!listener.initialized()) {
        return 2;
    }
    gobj = &listener;
    INSTALLSIGINTHANDLER(handlesig);
    listener.run();
    return 0;
}