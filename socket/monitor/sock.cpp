#include "sock.h"

#include <vector>

#include <sys/un.h>
#include <assert.h>

#include "fs.h"

namespace sun {
    namespace io {
        UnixServer::UnixServer(const std::string &path) {
            cleanup_ = Defer([this] { utility::Close(fd_); });
            ERRRET(remove(path.c_str()) == -1 && errno != ENOENT, , , 1, "Fail to remove %s", path.c_str());
            struct sockaddr_un sa{};
            sa.sun_family = AF_UNIX;
            strcpy(sa.sun_path, path.c_str());
            fd_ = socket(AF_UNIX, config_.type, 0);
            ERRRET(bind(fd_, SOCKADDR_EX(sa)) == -1, , { cleanup_.trigger(); }, 1, "bind");
            if (config_.type == SOCK_STREAM) {
                ERRRET(listen(fd_, config_.backlog) == -1, , { cleanup_.trigger(); }, 1, "listen");
            }
            LOGINFO("new UnixServer %s", path.c_str());
        }

        TcpipServer::TcpipServer(short port, bool lazy) {
            config_.port = port;
            if (!lazy) {
                initialize();
            }
        }

        TcpipServer &TcpipServer::initialize() {
            if (state_ == EndPoint::State::NOTHINGNESS) {
                cleanup_ = Defer([this] { utility::Close(fd_); });
                fd_ = socket(AF_INET, SOCK_STREAM, 0);
                if (config_.reuseaddr) {
                    SETSOCKOPT(fd_, SOL_SOCKET, SO_REUSEADDR, config_.reuseaddr, *this);
                }
                if (config_.reuseport) {
                    SETSOCKOPT(fd_, SOL_SOCKET, SO_REUSEPORT, config_.reuseport, *this);
                }
                struct sockaddr_in sa{.sin_family=AF_INET, .sin_port=htons(config_.port), .sin_addr={INADDR_ANY}};
                ERRRET(bind(fd_, SOCKADDR_EX(sa)) == -1, *this, , 1, "bind");
                ERRRET(listen(fd_, config_.backlog) == -1, *this, , 1, "listen");
                LOGINFO("new TcpipServer " SOCKADDR_FMT, SOCKADDR_OF(sa));
                state_ = State::INITIALIZED;
            }
            return *this;
        }

        void TcpipServer::enableSharePort() {
            config_.reuseport = 1;
        }

        UnixClient::UnixClient(const std::string &path) {
            cleanup_ = Defer([this] { utility::Close(fd_); });
            fd_ = socket(AF_UNIX, config_.type, 0);
            struct sockaddr_un sa{};
            sa.sun_family = AF_UNIX;
            strcpy(sa.sun_path, path.c_str());
            if (config_.timeout > 0) {
                struct timeval tv{config_.timeout, 0};
                SETSOCKOPT(fd_, SOL_SOCKET, SO_SNDTIMEO, tv,);
            }
            ERRRET(connect(fd_, SOCKADDR_EX(sa)) == -1, , , 1, "connect");
        }

        Poll::Entry::Entry(int epoll_handler, int fd, unsigned events) : fd_(fd) {
            if (Register(epoll_handler, fd, events) == 0) {
                cleanup_ = Defer([epoll_handler, fd] {
                    Unregister(epoll_handler, fd);
                    close(fd);
                });
            }
        }

        bool Poll::Entry::trigger(int fd, unsigned int events) const {
            if (fd == fd_) {
                if (events & EPOLLRDHUP && on_close) {
                    on_close(fd);
                } else if (events & EPOLLIN && on_read) {
                    on_read(fd);
                }
                return true;
            }
            return false;
        }

        int Poll::Entry::Register(int epoll_handler, int fd, unsigned events) {
            FUNCLOG("%d->%d", fd, epoll_handler);
            struct epoll_event ev{};//{.events=events, .data={.fd=fd_,},}; /* non-trivial designated initializers */
            ev.events = events;
            ev.data.fd = fd;
            ERRRET(epoll_ctl(epoll_handler, EPOLL_CTL_ADD, fd, &ev) == -1, -1, , 1, "epoll_ctl#EPOLL_CTL_ADD");
            return 0;
        }

        int Poll::Entry::Unregister(int epoll_handler, int fd) {
            FUNCLOG("%d<-%d", fd, epoll_handler);
            struct epoll_event ev{};//.data={.fd=fd_,},};
            ev.data.fd = fd;
            ERRRET(epoll_ctl(epoll_handler, EPOLL_CTL_DEL, fd, &ev) == -1, -1, , 1, "epoll_ctl#EPOLL_CTL_DEL");
            return 0;
        }

        Poll::Poll() {
            ERRRET((epoll_handler_ = epoll_create1(0)) == -1, , , 1, "epoll_create1");
            cleanup_ = Defer([this] { utility::Close(epoll_handler_); });
        }

        void Poll::registerEntry(int fd, unsigned int events, Entry::Callback on_read, Entry::Callback on_close) {
            Entry entry(epoll_handler_, fd, events);
            Entry::Callback on_close_hook = [on_close, this](int fd) {
                if (on_close) {
                    on_close(fd);
                }
                entry_map_.erase(fd);
            };
            entry.on_read.swap(on_read);
            entry.on_close.swap(on_close_hook);
            entry_map_[fd] = entry.move();
        }

        void Poll::loop() {
            registerEntry(terminator_.fd(), EPOLLIN, [this](int fd) { throw true; }, {}); // quit handler
            std::vector<struct epoll_event> events(config_.max_events);
            int nfds;
            try {
                for (;;) {
                    POLL(nfds, epoll_wait, epoll_handler_, &events[0], config_.max_events, config_.timeout);
                    for (int i = 0; i < nfds; ++i) {
                        auto &ev = events[i];
                        for (auto &it:entry_map_) {
                            if (it.second.trigger(ev.data.fd, ev.events)) {
                                break;
                            }
                        }
                    }
                }
            } catch (bool &) {
                LOGINFO("Quit loop");
            }
        }

        bool SendFd(int sock, int fd) {
            FUNCLOG("%d->%d", fd, sock);
            char buf[CMSG_SPACE(sizeof(fd))]{};
            char trivial{};
            struct iovec iov{.iov_base=&trivial, .iov_len=1,};
            struct cmsghdr *hdr;
            struct msghdr msg{};
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = buf;
            msg.msg_controllen = sizeof(buf);
            hdr = CMSG_FIRSTHDR(&msg);
            hdr->cmsg_len = CMSG_LEN(sizeof(fd));
            hdr->cmsg_level = SOL_SOCKET;
            hdr->cmsg_type = SCM_RIGHTS;
            *reinterpret_cast<int *>(CMSG_DATA(hdr)) = fd;
            ERRRET(sendmsg(sock, &msg, 0) == -1, false, , 1, "sendmsg");
            return true;
        }

        int RecvFd(int sock) {
            char buf[CMSG_SPACE(sizeof(int))];
            char trivial{};
            struct iovec iov{.iov_base=&trivial, .iov_len=1,};
            struct msghdr msg{};//{.msg_iov=&iov, .msg_iovlen=1, .msg_control=buf, .msg_controllen=sizeof(buf),};
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = buf;
            msg.msg_controllen = sizeof(buf);
            ERRRET(recvmsg(sock, &msg, MSG_WAITALL) == -1, -1, , 1, "recvmdg");
            auto hdr = CMSG_FIRSTHDR(&msg);
            assert(hdr
                   && hdr->cmsg_len == CMSG_LEN(sizeof(int))
                   && hdr->cmsg_level == SOL_SOCKET
                   && hdr->cmsg_type == SCM_RIGHTS);
            int fd = *reinterpret_cast<int *>(CMSG_DATA(hdr));
            FUNCLOG("%d<-%d", fd, sock);
            return fd;
        }
    }
}