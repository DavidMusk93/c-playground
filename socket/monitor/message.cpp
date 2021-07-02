#include "message.h"
#include "base.h"

#include <sys/socket.h>
#include <sys/uio.h>

#define MSGMAXLEN 4096

namespace sun {
    namespace io {
        const char *strmsgtype(int type) {
#define CASEMSGTYPE(x) case MSG##x:return "MSG" #x
            switch (type) {
                CASEMSGTYPE(REQUEST);
                CASEMSGTYPE(RESPONSE);
                CASEMSGTYPE(PING);
                CASEMSGTYPE(PONG);
                default:
                    return "UNKNOWN";
            }
#undef CASEMSGTYPE
        }

        struct header {
            int t;
            int l;
        };

        bool MsgRaw::read(int fd) {
            header hdr{};
            int nr;
            nr = (int) recv(fd, &hdr, sizeof hdr, MSG_WAITALL);
            if (nr != sizeof hdr) {
                return false;
            }
//    if(hdr.l>MSGMAXLEN){
//    }
            LOGDEBUG("@MSGRAW read %s,%d", strmsgtype(hdr.t), hdr.l);
            type = hdr.t;
            if (hdr.l <= 0) { /*without payload*/
                return true;
            }
            payload.resize(hdr.l);
            return recv(fd, &payload[0], hdr.l, MSG_WAITALL) == hdr.l;
        }

        bool MsgRaw::write(int fd) {
            struct iovec vec[2];
            header hdr{type, int(payload.size())};
            LOGDEBUG("@MSGRAW write %s,%d", strmsgtype(hdr.t), hdr.l);
            vec[0] = {&hdr, sizeof(hdr)};
            vec[1] = {&payload[0], payload.size()};
            return writev(fd, vec, 2) == int(sizeof(hdr) + payload.size());
        }

        bool MsgRequest::unpack(const char *data, const char *end, int &l) {
            Unpacker r(data, (int) (end - data));
            if (r.unpack(op) && r.unpack(arg)) {
//                l = 4 + 4;
                l = (int) (r.cursor() - data);
                return true;
            }
            return false;
        }

        std::string MsgRequest::pack() {
            Packer w;
            w.pack(op).pack(arg);
            return w.payload();
        }

        bool MsgRequest::unpack(Unpacker *r) {
            return r->unpack(op) &&
                   r->unpack(arg);
        }

        std::string MsgResponse::pack() {
            Packer w;
            w.pack(r1).pack(r2);
            return w.payload();
        }

        bool MsgResponse::unpack(const char *data, const char *end, int &l) {
            Unpacker r(data, int(end - data));
            if (r.unpack(r1) && r.unpack(r2)) {
                l = int(r.cursor() - data);
                return true;
            }
            return false;
        }

        bool MsgResponse::unpack(Unpacker *r) {
            return r->unpack(r1) &&
                   r->unpack(r2);
        }

        bool MsgHeartbeat::unpack(const char *data, const char *end, int &l) {
            Unpacker r(data, int(end - data));
            if (r.unpack(timestamp)) {
                l = int(r.cursor() - data);
                return true;
            }
            return false;
        }

        std::string MsgHeartbeat::pack() {
            Packer w;
            w.pack(timestamp);
            return w.payload();
        }

        bool MsgHeartbeat::unpack(Unpacker *r) {
            return r->unpack(timestamp);
        }

        std::string MsgCmd::pack() {
            Packer w;
            w.pack(op).pack(arg);
            return w.payload();
        }
    }
}
