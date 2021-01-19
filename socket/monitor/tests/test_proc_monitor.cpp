//
// Created by esgyn on 1/15/2021.
//

#include "proc_monitor.h"
#include "test.h"

static void OnExec(int pid) {
    LOGINFO("@%s %d", __func__, pid);
}

static void OnExit(int pid, int rc) {
    LOGINFO("@%s %d,%d", __func__, pid, rc);
}

MAIN() {
    sun::io::ProcMonitor pm;
    pm.on_process_exec = &OnExec;
    pm.on_process_exit = &OnExit;
    if (pm.connect() && pm.subscribe()) {
        pm.start();
        SIGBLOCK();
        pm.stop();
    }
}