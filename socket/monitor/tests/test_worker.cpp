#include <fstream>

#include "test.h"
#include "worker.h"
#include "common.h"

static sun::Worker *g_handler;

static void SigHandler(int sig) {
    if (g_handler) {
        g_handler->pollInstance().quit();
    }
}

MAIN() {
    int pid;
    std::ifstream ifs(COORDINATOR_PIDFILE);
    if (!ifs.is_open()) {
        LOGERROR("Coordinator pidfile not exists");
        return 1;
    }
    ifs >> pid;
    if (kill(pid, 0) != 0) {
        LOGERROR("Invalid Coordinator process");
        return 2;
    }
    INSTALLSIGINTHANDLER(&SigHandler);
    sun::Worker worker;
    g_handler = &worker;
    worker.loop();
}