//
// Created by Steve on 4/23/2020.
//

#include "utility.h"

#include "macro.h"

int tty_set_char_break(int fd, struct termios *pre) {
    struct termios t;
    ERROR_RETURN(tcgetattr(fd, &t) == -1, -1);
    if (pre) {
        *pre = t;
    }
    /** local modes:
     * ISIG   When any of the characters INTR, QUIT, SUSP, or DSUSP are
     *        received, generate the corresponding signal.
     * ICANON Enable canonical mode.
     * ECHO   Echo input characters.
     */
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_lflag |= ISIG;
    /** input modes:
     * ICRNL   Translate carriage return to newline on input (unless IGNCR is
     *         set).
     */
    t.c_iflag &= ~ICRNL;
    /** special characters:
     * VMIN   Minimum number of characters for noncanonical read.
     * VTIME  Timeout in deciseconds for noncannonical read.
     */
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    ERROR_RETURN(tcsetattr(fd, TCSAFLUSH, &t) == -1, -1);
    return 0;
}

int tty_set_raw(int fd, struct termios *pre) {
    struct termios t;
    ERROR_RETURN(tcgetattr(fd, &t) == -1, -1);
    if (pre) {
        *pre = t;
    }
    /* Noncanonical mode, disable signals, extended input processing, and echoing */
    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
    /* Disable special handling of CR, NL, and BREAK. No 8th-bit stripping or
     * parity error handling. Disable START/STOP output flow control. */
    t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
                   INPCK | ISTRIP | IXON | PARMRK);
    /* Disable all output processing. */
    t.c_oflag &= ~OPOST;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    ERROR_RETURN(tcsetattr(fd, TCSAFLUSH, &t) == -1, -1);
    return 0;
}