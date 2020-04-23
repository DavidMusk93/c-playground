//
// Created by Steve on 4/16/2020.
//

#ifndef CPP4FUN_COMMON_H
#define CPP4FUN_COMMON_H

#define SETSOCKOPT(sock, level, name, val, errorCode) do {\
    if(setsockopt(sock,level,name,&val,sizeof val)<0){\
      perror("setsockopt(" #name ") error");\
      close(sock);\
      return errorCode;\
    }\
} while (0)

#define MAKE_SOCKADDR_IN(var, addr, port) /* addr, port must be in network order */\
struct sockaddr_in var = {};\
var.sin_family = AF_INET;\
var.sin_addr.s_addr = (addr);\
var.sin_port = (port);

#define SOCKADDR_EXT(addr) (struct sockaddr *)&addr, sizeof addr

#define GROUP_IP "228.67.43.91"
#define GROUP_PORT 15847

#endif //CPP4FUN_COMMON_H
