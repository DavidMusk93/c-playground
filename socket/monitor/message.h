#ifndef MONITOR_MESSAGE_H
#define MONITOR_MESSAGE_H

namespace sun {
    enum class MessageType : int {
        PING,
        PONG,
        BUSY,
        IDLE,
        UNKNOWN,
    };

    struct Ping {
        double timestamp;
    };
    struct Pong {
        double timestamp;
    };
    struct Busy {
        long reserved;
    };
    struct Idle {
        long reserved;
    };

    struct Message {
        MessageType type;
        int pid;
        union {
            Ping ping;
            Pong pong;
            Busy busy;
            Idle idle;
        };
    };
}

#endif //MONITOR_MESSAGE_H
