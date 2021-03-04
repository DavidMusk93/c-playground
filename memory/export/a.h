#ifndef C_PLAYGROUND_A_H
#define C_PLAYGROUND_A_H

class A {
public:
    A() {
        a = 5;
#ifdef DEBUG
        p = this;
#endif
    }

    int a;
#ifdef DEBUG /*@BAD_DESIGN it is danger to use macro in library interface*/
    void *p;
#endif

    void dump();
};

#endif //C_PLAYGROUND_A_H
