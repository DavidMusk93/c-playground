#include "macro.h"

/*                -7-6-5-4-3-2-1-0*/
#define LONGMAX 0x7fffffffffffffff
#define LONGMIN 0x8000000000000000

struct Counter {
private:
    long a[4];
public:
    void dump() {
        LOG("%#lx,%#lx,%#lx,%#lx", a[0], a[1], a[2], a[3]);
    }

    Counter &operator++() {
        AddOne(dimension_of(a) - 1);
        return *this;
    }

    Counter &operator--() {
        SubOne(dimension_of(a) - 1);
        return *this;
    }

    Counter &operator+=(long v) {
        auto v0 = a[3];
        a[3] += v;
        if (v0 > 0 && v > 0) {
            if (v0 > a[3] && v > a[3]) {
                a[3] &= ~(1L << 63);
                AddOne(2);
            }
        } else if (v0 < 0 && v < 0) {
            if (a[3] > v0 && a[3] > v) {
                SubOne(2);
            }
        }
        return *this;
    }

protected:
    void AddOne(int i) {
        ++a[i];
        if (a[i] == 0) {
            ++a[--i];
            if (i) {
                AddOne(i);
            }
        }
    }

    void SubOne(int i) {
        --a[i];
        if (a[i] == 0) {
            --a[--i];
            if (i) {
                SubOne(i);
            }
        }
    }
};

MAIN() {
    Counter counter{};
    ++counter;
    counter.dump();
    ++counter;
    ++counter;
    counter.dump();
    counter += LONGMAX;
    counter.dump();
    LOG("@@@@@@");
    Counter c2{};
    --c2;
    --c2;
    c2 += LONGMIN;
    c2.dump();
}