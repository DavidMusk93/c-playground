#include "message.h"
#include "sock.h"
#include "test.h"

#include <sys/poll.h>

static bool parseInput(const char *buf, int &a1, int &a2) {
    int nr = sscanf(buf, "%d %d", &a1, &a2);
    return nr == 2;
}

static std::string v2s(const std::vector<char> &v) {
    std::stringstream ss;
    char buf[8];
    for (auto &&c:v) {
        sprintf(buf, "%#x,", unsigned(c));
        ss << buf;
    }
    return ss.str();
}

MAIN_EX(argc, argv) {
    if (argc < 3) {
        LOGERROR("usage: %s ip port", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    auto port = (short) atoi(argv[2]);
    using namespace sun::io;
    TcpipClient client(ip, port);
    if (!client.valid()) {
        LOGERROR("invalid arguments");
        return 2;
    }
    int sock = client.fd();
    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO, pfds[0].events = POLLIN;
    pfds[1].fd = sock, pfds[1].events = POLLIN;
    int rc;
    for (;;) {
        POLL(rc, poll, &pfds[0], 2, -1);
        if (pfds[0].revents == POLLIN) {
            char buf[64];
            if (!fgets(buf, sizeof buf, stdin)) {
                LOGERROR("@FATAL read from stdin error");
                break;
            }
            int a, b;
            if (parseInput(buf, a, b)) {
                LOGINFO("input %d,%d", a, b);
            } else {
                LOGERROR("invalid input");
            }
            MsgRequest res(a, b);
            MsgRaw raw(MSGREQUEST);
            raw.payload = res.pack();
            raw.write(sock);
        }
        if (pfds[1].revents == POLLIN) {
            MsgRaw raw;
            if (!raw.read(sock)) {
                LOGERROR("@FATAL server may abort, quit");
                break;
            }
            LOGINFO("message type: %s", strmsgtype(raw.type));
            Unpacker r(raw.payload);
            switch (raw.type) {
                case MSGRESPONSE: {
                    MsgResponse res;
                    if (res.unpack(&r)) {
                        LOGINFO("response: %s,%s", res.r1.c_str(), v2s(res.r2).c_str());
                    } else {
                        LOGERROR("unpack error: %p:%p", ((void **) &r)[0], ((void **) &r)[1]);
                    }
                    break;
                }
                case MSGPING: {
                    raw.type = MSGPONG;
                    raw.write(sock);
                    break;
                }
            }
        }
    }
    return 0;
}