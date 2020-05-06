//
// Created by Steve on 4/25/2020.
//

#ifndef C4FUN_MASTER_OPEN_H
#define C4FUN_MASTER_OPEN_H

#include <stddef.h>

#define FD_OP_CHECK(error, fd, cache, code) \
if(error){\
    cache=errno;\
    close(fd); /* Might change 'errno' */\
    errno=cache;\
    return code;\
}

int pty_master_open(char *slave_name, size_t sn_len);

#endif //C4FUN_MASTER_OPEN_H
