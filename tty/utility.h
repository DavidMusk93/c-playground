//
// Created by Steve on 4/23/2020.
//

#ifndef C4FUN_UTILITY_H
#define C4FUN_UTILITY_H

#include <termios.h>

int tty_set_char_break(int fd, struct termios *pre);

int tty_set_raw(int fd, struct termios *pre);

#endif //C4FUN_UTILITY_H
