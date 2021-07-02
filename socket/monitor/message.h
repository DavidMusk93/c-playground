#ifndef FORWARDER_MESSAGE_H
#define FORWARDER_MESSAGE_H

#include <vector>
#include <string>
#include <sstream>

#include <string.h>

namespace sun {
    namespace io {
        class Unpacker;

        class unpackable {
        public:
            virtual bool unpack(const char *data, const char *end, int &l) = 0;

            virtual bool unpack(Unpacker *r) = 0;
        };

        class packable {
        public:
            virtual std::string pack() = 0;
        };

        class Unpacker {
        public:
            Unpacker(const char *data, const char *end) : data_(data), end_(end) {}

            Unpacker(const char *data, int len) : data_(data), end_(data + len) {}

            explicit Unpacker(const std::string &v) : Unpacker(v.data(), v.data() + v.size()) {}

            template<class T>
            bool unpack(T &t) {
//                if(dynamic_cast<unpackable*>(&t)){
//                }
                if (data_ + sizeof(T) > end_) {
                    return false;
                }
                memcpy((void *) &t, data_, sizeof(T));
                data_ += sizeof(T);
                return true;
            }

//            template<>
            bool unpack(std::string &t) {
                int l;
                if (unpack(l)) {
                    if (l == 0) {
                        return true;
                    }
                    if (data_ + l + 1 > end_) {
                        return false;
                    }
                    std::string s(data_, data_ + l);
                    t.swap(s);
                    data_ += l + 1;
                    return true;
                }
                return false;
            }

//            template<>
            bool unpack(std::vector<char> &t) {
                int l;
                if (unpack(l)) {
                    if (l == 0) {
                        return true;
                    }
                    if (data_ + l > end_) {
                        return false;
                    }
                    std::vector<char> v(data_, data_ + l);
                    t.swap(v);
                    data_ += l;
                    return true;
                }
                return false;
            }

            bool unpack(unpackable *obj) {
                int l{};
                if (obj->unpack(data_, end_, l)) {
                    data_ += l;
                    return true;
                }
                return false;
            }

            const char *cursor() const {
                return data_;
            }

        private:
            const char *data_;
            const char *end_;
        };

        class Packer {
        public:
            template<class T>
            Packer &pack(const T &t) {
                auto p = reinterpret_cast<const char *>(&t);
                std::string s(p, p + sizeof(T));
//                s.resize(sizeof(T)); /*expect SSO(Small String Optimization)*/
//                memcpy(&s[0], &t, sizeof(T));
                ss_ << s;
                return *this;
            }

//            template<>
            Packer &pack(const std::string &t) {
                if (t.empty()) {
                    pack(int(0));
                } else {
                    pack(int(t.size()));
                    ss_ << t;
                    pack(char(0)); /*redundant?*/
                }
                return *this;
            }

//            template<>
            Packer &pack(const std::vector<char> &t) {
                if (t.empty()) {
                    pack(int(0));
                } else {
                    std::string s(t.begin(), t.end());
                    pack(int(t.size()));
                    ss_ << s;
                }
                return *this;
            }

            Packer &pack(packable *obj) {
                ss_ << obj->pack();
                return *this;
            }

            std::string payload() {
                return ss_.str();
            }

        private:
            std::stringstream ss_;
        };

#define MSGTYPE(x) MSG##x
#define MSGREQ(x) struct Msg##x:unpackable
#define MSGRES(x) struct Msg##x:packable
#define MSGBOTH(x) struct Msg##x:unpackable,packable

        enum MsgType {
            MSGTYPE(NOTHING),
            MSGTYPE(REQUEST),
            MSGTYPE(RESPONSE),
            MSGTYPE(PING),
            MSGTYPE(PONG),
            MSGTYPE(CMD),
        };

        extern const char *strmsgtype(int type);

        MSGBOTH(Request) {
            int op;
            int arg;

            MsgRequest() : MsgRequest(0, 0) {}

            MsgRequest(int op, int arg) : op(op), arg(arg) {}

            std::string pack() override;

            bool unpack(const char *data, const char *end, int &l) override;

            bool unpack(Unpacker *r) override;
        };

        MSGBOTH(Response) {
            std::string r1;
            std::vector<char> r2;

            MsgResponse() = default;

            MsgResponse(std::string r1, std::vector<char> r2) : r1(std::move(r1)), r2(std::move(r2)) {}

            std::string pack() override;

            bool unpack(const char *data, const char *end, int &l) override;

            bool unpack(Unpacker *r) override;
        };

        MSGBOTH(Heartbeat) {
            double timestamp;

            MsgHeartbeat() : timestamp(-1) {}

            explicit MsgHeartbeat(double ts) : timestamp(ts) {}

            std::string pack() override;

            bool unpack(const char *data, const char *end, int &l) override;

            bool unpack(Unpacker *r) override;
        };

        MSGRES(Cmd) {
            int op;
            std::string arg;

            MsgCmd(int op, std::string arg) : op(op), arg(std::move(arg)) {}

            std::string pack() override;
        };

        struct MsgRaw {
            int type;
            std::string payload;

            explicit MsgRaw(int type = MSGNOTHING) : type(type) {}

            MsgRaw(MsgRaw &&msg) noexcept {
                type = msg.type;
                payload.swap(msg.payload);
            }

            MsgRaw &operator=(MsgRaw &&msg) noexcept {
                type = msg.type;
                payload.swap(msg.payload);
                return *this;
            }

            MsgRaw &&move() {
                return static_cast<MsgRaw &&>(*this);
            }

            bool read(int fd);

            bool write(int fd);
        };
    }
}


#endif //FORWARDER_MESSAGE_H
