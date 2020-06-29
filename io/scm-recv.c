//
// Created by Steve on 6/28/2020.
//

#include "scm.h"

#include <assert.h>

MAIN_EX(argc, argv) {
    int data, fd, lfd, sfd, optval, opt;
    ssize_t nr;
    int use_datagram_socket = 0;
    /* The 'msg_name' field can be set to pointer to a buffer where
     * the kernel will place the address of the peer socket. */
    struct msghdr msg = {};
    struct iovec iov;
    struct ucred rcred, scred;
    char buf[CMSG_SPACE(PAYLOAD_SIZE)];
    struct cmsghdr *cmsgp;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd': {
                use_datagram_socket = 1;
                break;
            }
            default:
                LOG("%s [-d]\n"
                    "       -d    use datagram socket", argv[0]);
                return 0;
        }
    }
    ERROR_RETURN(remove(SOCK_PATH) == -1 && errno != ENOENT, 1, , "remove: %m");
    struct sockaddr_un server = {
            .sun_family=AF_UNIX,
            .sun_path=SOCK_PATH,
    };
    fd = socket(AF_UNIX, use_datagram_socket ? SOCK_DGRAM : SOCK_STREAM, 0);
    ERROR_RETURN(bind(fd, SOCKADDR_EX(server)) == -1, 1, { close(fd); }, "bind: %m");
    if (use_datagram_socket) {
        sfd = fd;
    } else {
        lfd = fd;
        ERROR_RETURN(listen(lfd, 5) == -1, 1, { close(lfd); }, "listen: %m");
        ERROR_RETURN((sfd = accept(lfd, 0, 0)) == -1, 1, { close(lfd); }, "accept: %m");
    }
#define CLEAN() \
do{\
    if(!use_datagram_socket){\
        close(lfd);\
    }\
    close(sfd);\
}while(0)
    optval = 1;
#ifdef TEST_SCM_CREDENTIALS
    /* use SO_PASSCRED socket option to receive credentials (this option would modify control message) */
    ERROR_RETURN(setsockopt(sfd,SOL_SOCKET,SO_PASSCRED,&optval,sizeof(optval))==-1, 1, {CLEAN();}, "setsockopt: %m");
#endif
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(data);
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    nr = recvmsg(sfd, &msg, 0);
    cmsgp = CMSG_FIRSTHDR(&msg);
    assert(cmsgp && cmsgp->cmsg_len == CMSG_LEN(PAYLOAD_SIZE));
    assert(cmsgp->cmsg_level == SOL_SOCKET);
    assert(cmsgp->cmsg_type == CMSG_TYPE);
#ifdef TEST_SCM_CREDENTIALS
    memcpy(&rcred,CMSG_DATA(cmsgp),PAYLOAD_SIZE);
    DUMP_CREDS("received credentials",&rcred);
#else
    memcpy(&fd, CMSG_DATA(cmsgp), PAYLOAD_SIZE);
    LOG("received FD %d", fd);
    LOG("FILE CONTENT @BEGIN");
    for (;;) {
        char blob[1024];
        ERROR_RETURN((nr = read(fd, blob, sizeof(blob))) == -1, 1, { close(fd); }, "read: %m");
        if (!nr) {
            break;
        }
        write(1, blob, nr);
    }
    close(fd);
    LOG("FILE CONTENT @END");
#endif
    /* not available for datagram */
    socklen_t len = sizeof(struct ucred);
    ERROR_RETURN(getsockopt(sfd, SOL_SOCKET, SO_PEERCRED, &scred, &len) == -1, 1, { CLEAN(); }, "getsockopt: %m");
    DUMP_CREDS("credentials from SO_PEERCRED", &scred);
    CLEAN();
    return 0;
#undef CLEAN
}