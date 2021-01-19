#ifndef MONITOR_MESSAGE_H
#define MONITOR_MESSAGE_H

namespace sun {
    enum class MessageType : int {
        PING,
        PONG,
        BUSY,
        IDLE,
    };

    struct Ping {
        long timestamp;
    };
    struct Pong {
        long timestamp;
    };
    struct Busy {
        long reserved;
    };
    struct Idle {
        long reserved;
    };

    struct Message {
        MessageType type;
        union {
            Ping ping;
            Pong pong;
            Busy busy;
            Idle idle;
        };
    };
}

#endif //MONITOR_MESSAGE_H
