//
// Created by esgyn on 1/15/2021.
//

#ifndef MONITOR_SOCK_H
#define MONITOR_SOCK_H

#include <string>
#include <unordered_map>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "base.h"
#include "runner.h"

#define SOCKADDRPTR(ptr) (struct sockaddr*)(ptr)
#define SOCKADDR_EX(x) SOCKADDRPTR(&x),sizeof(x)
#define SOCKADDR_FMT "%s#%d"
#define SOCKADDR_OF(x) inet_ntoa((x).sin_addr),ntohs((x).sin_port)

#define SETSOCKOPT(sock, level, name, val, code) \
ERRRET(setsockopt(sock,level,name,&val,sizeof(val))!=0,code,,1,"setsockopt");

namespace sun {
    namespace io {
        class EndPoint : public nocopy {
        public:
            virtual ~EndPoint() = default;

            explicit operator int() const {
                return fd_;
            };

            virtual int transferOwnership() {
                cleanup_.cancel();
                return fd_;
            };

        protected:
            int fd_{-1};
            Defer cleanup_;
        };

        class UnixServer : public EndPoint {
        public:
            explicit UnixServer(const std::string &path);

            ~UnixServer() override = default;

        private:
            struct {
                int backlog{5};
                int type{SOCK_STREAM};
            } config_;
        };

        class UnixClient : public EndPoint {
        public:
            explicit UnixClient(const std::string &path);

            ~UnixClient() override = default;

        private:
            struct {
                int timeout{2000}; // seconds
                int type{SOCK_STREAM};
            } config_;
        };

        class TcpipServer : public EndPoint {
        public:
            explicit TcpipServer(short port);

            ~TcpipServer() override = default;

        private:
            struct {
                int backlog{5};
                int reuseaddr{1};
                int reuseport{0};
            } config_;
        };

        class Poll : public nocopy {
        public:
            class Entry : public nocopy {
            public:
                using Callback = std::function<void(int)>;

                Entry() : fd_(-1) {}

                Entry(int epoll_handler, int fd, unsigned events);

                Entry(Entry &&entry) noexcept: fd_(-1) {
                    *this = entry.move();
                }

                Entry &operator=(Entry &&entry) noexcept {
                    on_read.swap(entry.on_read);
                    on_close.swap(entry.on_close);
                    std::swap(fd_, entry.fd_);
                    cleanup_.swap(entry.cleanup_);
                    return *this;
                }

                ~Entry() = default;

                bool trigger(int fd, unsigned events) const;

                Entry &&move() {
                    return static_cast<Entry &&>(*this);
                }

            protected:
                static int Register(int epoll_handler, int fd, unsigned events);

                static int Unregister(int epoll_handler, int fd);

            public:
                Callback on_read;
                Callback on_close;
            private:
                int fd_;
                Defer cleanup_;
            };

            Poll();

            ~Poll() = default;

            explicit operator int() const {
                return epoll_handler_;
            }

            void registerEntry(int fd, unsigned events, Entry::Callback on_read, Entry::Callback on_close);

            void loop();

            void quit() {
                terminator_.trigger();
            }

        private:
            int epoll_handler_;
            Defer cleanup_;
            Terminator terminator_;
            std::unordered_map<int, Entry> entry_map_;
            struct {
                int max_events{100};
                int timeout{60000};
            } config_;
        };

        bool SendFd(int sock, int fd);

        int RecvFd(int sock);
    }

    namespace utility {
        void close(int &fd);
    }
}

#endif //MONITOR_SOCK_H
