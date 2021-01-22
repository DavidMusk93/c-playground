#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <poll.h>

#include "proc_monitor.h"

namespace sun {
    namespace io {
        bool ProcMonitor::connect() {
            int sock;
            struct sockaddr_nl sa;
            Defer on_success([this, &sock] { nl_sock_ = sock; }), on_failure;
            sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
            ERRRET(sock == -1, false, , 1, "Fail to create netlink socket");
            on_failure = Defer([&sock] { close(sock); });
            sa.nl_family = AF_NETLINK;
            sa.nl_groups = CN_IDX_PROC;
            sa.nl_pid = sun::util::GetPid();
            ERRRET(bind(sock, (struct sockaddr *) &sa, sizeof(sa)) == -1, false, , 1, "bind");
            on_failure.cancel();
            return true;
        }

        bool ProcMonitor::subscribe() {
            struct __ATTR(aligned(NLMSG_ALIGNTO)) {
                struct nlmsghdr hdr;
                struct __ATTR(__packed__) {
                    struct cn_msg body;
                    enum proc_cn_mcast_op op;
                };
            } msg{};
            auto &hdr = msg.hdr;
            hdr.nlmsg_len = sizeof(msg);
            hdr.nlmsg_pid = sun::util::GetPid();
            hdr.nlmsg_type = NLMSG_DONE;
            msg.body.id.idx = CN_IDX_PROC;
            msg.body.id.val = CN_VAL_PROC;
            msg.body.len = sizeof(enum proc_cn_mcast_op);
            msg.op = config_.enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;
            ERRRET(send(nl_sock_, &msg, sizeof(msg), 0) == -1, false, , 1, "netlink send");
            return true;
        }

        void ProcMonitor::Run() {
            struct __ATTR(aligned(NLMSG_ALIGNTO)) {
                struct nlmsghdr hdr;
                struct __ATTR(__packed__) {
                    struct cn_msg body;
                    struct proc_event event;
                };
            } msg{};
            int nfds, nr;
            Defer on_done([this] { close(nl_sock_); });
            struct pollfd pfds[2]{};
            pfds[0] = {.fd=quitHandler(), .events=POLLIN,};
            pfds[1] = {.fd=nl_sock_, .events=POLLIN,};
            for (;;) {
                POLL(nfds, poll, pfds, 2, config_.timeout);
                if (pfds[0].revents & POLLIN) {
                    break;
                }
                if (pfds[1].revents & POLLIN) {
                    nr = recv(nl_sock_, &msg, sizeof(msg), 0);
                    ERRRET(nr == -1, , { if (errno == EINTR)continue; }, 1, "netlink recv");
                    auto what = msg.event.what;
                    if (what == proc_event::PROC_EVENT_NONE) {
                        LOGINFO("set mcast listen ok");
                    } else if (what == proc_event::PROC_EVENT_EXEC && on_process_exec) {
                        on_process_exec(msg.event.event_data.exec.process_pid);
                    } else if (what == proc_event::PROC_EVENT_EXIT && on_process_exit) {
                        auto exit = msg.event.event_data.exit;
                        on_process_exit(exit.process_pid, exit.exit_code);
                    }
                }
            }
        }
    }
}