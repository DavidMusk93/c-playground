//
// Created by Steve on 4/16/2020.
//

#ifndef CPP4FUN_COMMON_H
#define CPP4FUN_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

#define SETSOCKOPT(sock, level, name, val, ret_code) do {\
    if(setsockopt(sock,level,name,&val,sizeof val)<0){\
      perror("setsockopt(" #name ") error");\
      close(sock);\
      return ret_code;\
    }\
} while (0)

#define MAKE_SOCKADDR_IN(var, addr, port) /* addr, port must be in network order */\
struct sockaddr_in var = {};\
var.sin_family = AF_INET;\
var.sin_addr.s_addr = (addr);\
var.sin_port = (port)

#define SOCKADDR_EX(addr) (struct sockaddr *)&addr, sizeof addr

#define GROUP_IP "228.67.43.91"
#define GROUP_PORT 15847

#define ERROR_RETURN(error, code, ext_block, fmt, ...) do{\
    if(error){\
        printf(fmt "\n", ##__VA_ARGS__);\
        ext_block\
        return code;\
    }\
} while (0)

#define RETRIEVE_ADDR(x) (((struct sockaddr_in *)(x))->sin_addr)

bool sock_join_group(int sock, in_addr_t group, in_addr_t recv_if);

bool sock_make_nonblock(int sock);

struct in_addr my_ip(int sock, const char *if_name);

#endif //CPP4FUN_COMMON_H
