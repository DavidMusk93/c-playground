//
// Created by esgyn on 1/18/2021.
//

#include "test.h"
#include "coordinator.h"

static sun::Coordinator *g_handler;

static void SigHandler(int sig) {
    if (g_handler) {
        g_handler->pollInstance().quit();
    }
}

MAIN() {
    sun::Coordinator coordinator;
    g_handler = &coordinator;
    INSTALLSIGINTHANDLER(&SigHandler);
    coordinator.loop();
}