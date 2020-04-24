//
// Created by Steve on 4/23/2020.
//

#include "utility.h"

#include "macro.h"

int tty_set_char_break(int fd, struct termios *old) {
    struct termios t;
    ERROR_RETURN(tcgetattr(fd, &t) == -1, -1);
    if (old) {
        *old = t;
    }
    /** local modes:
     * ISIG   When any of the characters INTR, QUIT, SUSP, or DSUSP are
     *        received, generate the corresponding signal.
     */
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_lflag |= ISIG;
    /** input modes:
     * ICANON Enable canonical mode.
     * ECHO   Echo input characters.
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