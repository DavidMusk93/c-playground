//
// Created by Steve on 6/28/2020.
//

#ifndef C4FUN_SCM_H
#define C4FUN_SCM_H
#define _GNU_SOURCE

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define EXTENDED_ER

#include "macro.h"

#define SOCK_PATH "scm-cred"

#define DUMP_CREDS(msg, x) LOG(msg " pid=%ld,uid=%ld,gid=%ld",(long)(x)->pid,(long)(x)->uid,(long)(x)->gid);

#define TEST_SCM_RIGHTS
#if defined(TEST_SCM_CREDENTIALS)
#define PAYLOAD_TYPE struct ucred
#define CMSG_TYPE SCM_CREDENTIALS
#elif defined(TEST_SCM_RIGHTS)
#define PAYLOAD_TYPE int
#define CMSG_TYPE SCM_RIGHTS
#else
#error "TEST TYPE IS NOT DEFINED"
#endif
#define PAYLOAD_SIZE sizeof(PAYLOAD_TYPE)

#endif //C4FUN_SCM_H
