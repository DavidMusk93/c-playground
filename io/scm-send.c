//
// Created by Steve on 6/28/2020.
//

#include "scm.h"

MAIN_EX(argc, argv) {
    int data = 312, sfd, opt, fd;
    ssize_t ns;
    int use_datagram_socket, no_explicit_creds;
    char buf[CMSG_SPACE(PAYLOAD_SIZE)] = {};
    struct cmsghdr *cmsgp;

    use_datagram_socket = 0, no_explicit_creds = 0;
    while ((opt = getopt(argc, argv, "dn")) != -1) {
        switch (opt) {
            case 'd': {
                use_datagram_socket = 1;
                break;
            }
#if defined(TEST_SCM_CREDENTIALS)
                case 'n':{
                    no_explicit_creds=1;
                    break;
                }
#endif
            default:
#if defined(TEST_SCM_CREDENTIALS)
                LOG("%s [-d] [-n] [data [PID [UID [GID]]]]\n"
                    "       -d    use datagram socket\n"
                    "       -n    do not construct explicit credentials structure",argv[0]);
#else
                LOG("%s [-d] file\n"
                    "       -d    use datagram socket", argv[0]);
#endif
                return 0;
        }
    }
#if defined(TEST_SCM_CREDENTIALS)
    data=argc>optind?atoi(argv[optind]):312;
#else
    ERROR_RETURN(argc != optind + 1, 1, , "%s [-d] file", argv[0]);
    ERROR_RETURN((fd = open(argv[optind], O_RDONLY)) == -1, 1, , "open: %m");
#endif
    LOG("sending data=%d", data);
    struct iovec iov = {
            .iov_base=&data,
            .iov_len=sizeof(data),
    };
#define CMSG_SET_BUFFER(msg, buf, ptr) \
msg.msg_control=buf;\
msg.msg_controllen=sizeof(buf);\
ptr=CMSG_FIRSTHDR(&msg);/* same as ptr=buf; */\
ptr->cmsg_len=CMSG_LEN(PAYLOAD_SIZE);\
ptr->cmsg_level=SOL_SOCKET;\
ptr->cmsg_type=CMSG_TYPE
    /* connect() sets a default outgoing address for datagrams */
    struct msghdr msg = {};
#ifdef TEST_SCM_CREDENTIALS
    if(!no_explicit_creds){
        CMSG_SET_BUFFER(msg,buf,cmsgp);
        struct ucred creds;
#define UCRED_MEMBER_SET(x,default,index) \
do{\
    creds.x=default;\
    if(argc>index&&strcmp(argv[index],"-")!=0){\
        creds.x=atoi(argv[index]);\
    }\
}while(0)
        UCRED_MEMBER_SET(pid,getpid(),optind+1);
        UCRED_MEMBER_SET(uid,getuid(),optind+2);
        UCRED_MEMBER_SET(gid,getgid(),optind+3);
#undef UCRED_MEMBER_SET
        DUMP_CREDS("send credentials",&creds);
        memcpy(CMSG_DATA(cmsgp),&creds,sizeof(struct ucred));
    }
#else
    CMSG_SET_BUFFER(msg, buf, cmsgp);
    memcpy(CMSG_DATA(cmsgp), &fd, sizeof(int));
#endif
#undef CMSG_SET_BUFFER
    sfd = socket(AF_UNIX, use_datagram_socket ? SOCK_DGRAM : SOCK_STREAM, 0);
    struct sockaddr_un server = {
            .sun_family=AF_UNIX,
            .sun_path=SOCK_PATH,
    };
    ERROR_RETURN(connect(sfd, SOCKADDR_EX(server)) == -1, 1, { close(sfd); }, "connect: %m");
    ERROR_RETURN((ns = sendmsg(sfd, &msg, 0)) == -1, 1, { close(sfd); }, "sendmsg: %m");
    LOG("sendmsg() returned %ld", (long) ns);
    close(sfd);
#ifdef TEST_SCM_RIGHTS
    close(fd);
#endif
    return 0;
}