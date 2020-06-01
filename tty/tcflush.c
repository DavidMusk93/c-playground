//
// Created by Steve on 6/1/2020.
// CELEBT05, not work on Ubuntu
// @ref
// https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.4.0/com.ibm.zos.v2r4.bpxbd00/rttcfu.htm
// https://www.ibm.com/support/knowledgecenter/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rtmkn.htm
//

#define _POSIX_SOURCE

#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char Master[] = "master.tty";
    char Slave[] = "slave.tty";
    char text1[] = "string that will be flushed from buffer";
    char text2[] = "string that will not be flushed from buffer";
    char data[80];
    int master, slave;

    if (mknod(Master, S_IFCHR | S_IRUSR | S_IWUSR, 0x00010000 + 10) != 0)
        perror("mknod() error for master tty");
    else {
        if (mknod(Slave, S_IFCHR | S_IRUSR | S_IWUSR, 0x00020000 + 10) != 0)
            perror("mknod() error for slave tty");
        else {
            if ((master = open(Master, O_RDWR | O_NONBLOCK)) < 0)
                perror("open() error for master tty");
            else {
                if ((slave = open(Slave, O_RDWR | O_NONBLOCK)) < 0)
                    perror("open() error for slave tty");
                else {
                    if (write(slave, text1, strlen(text1) + 1) == -1)
                        perror("write() error");
                    else if (tcflush(slave, TCOFLUSH) != 0)
                        perror("tcflush() error");
                    else {
                        puts("first string is written and tty flushed");
                        puts("now writing string that will not be flushed");
                        if (write(slave, text2, strlen(text2) + 1) == -1)
                            perror("write() error");
                        else if (read(master, data, sizeof(data)) == -1)
                            perror("read() error");
                        else printf("read '%s' from the tty\n", data);
                    }
                    close(slave);
                }
                close(master);
            }
            unlink(Slave);
        }
        unlink(Master);
    }
}