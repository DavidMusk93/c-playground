//
// Created by esgyn on 1/15/2021.
//

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
    };
    struct Idle {
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
