//
// Created by Steve on 11/18/2020.
//

#include "macro.h"

#define USE_ASM 1

using binmap_t=unsigned;
using UInt32=unsigned;
namespace NAHeap{
    using bindex_t=unsigned;
    bindex_t bit2idx(binmap_t x);
    bindex_t computeTreeIndex(size_t size);
}

#define TREEBIN_SHIFT 8
#define NTREEBINS 32

// Return index corresponding to given bit.  This function assumes
// that the caller has isolated a single bit and that exactly one
// bit is set.
inline NAHeap::bindex_t
NAHeap::bit2idx(binmap_t x)
{
//#if defined(i386)
#if USE_ASM
    UInt32 ret;
  __asm__("bsfl %1,%0\n\t"
          : "=r"(ret)
          : "rm"(x));
  return (NAHeap::bindex_t)ret;
#else
    // Set all bits right of the set bit.
    x--;

    // Quickly count the number of set bits using a well known
    // population count algorithm.
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);

    return x & 0x3f;
#endif
}

inline NAHeap::bindex_t
NAHeap::computeTreeIndex(size_t size)
{
    size_t x = size >> TREEBIN_SHIFT;
    if (x == 0)
        return 0;
    else if (x > 0xFFFF)
        return NTREEBINS - 1;
    else
    {
        UInt32 k;
//#if defined(i386)
#if USE_ASM
        __asm__("bsrl %1,%0\n\t"
            : "=r"(k)
            : "rm"(x));
        LOG("k:%d",k);
#else
        UInt32 y = (UInt32)x;
        UInt32 n = ((y - 0x100) >> 16) & 8;
        k = (((y <<= n) - 0x1000) >> 16) & 4;
        n += k;
        n += k = (((y <<= k) - 0x4000) >> 16) & 2;
        k = 14 - n + ((y <<= k) >> 15);
#endif

        return (k << 1) + ((size >> (k + (TREEBIN_SHIFT - 1)) & 1));
    }
}

MAIN(){
    unsigned k=24;
    LOG("%u,%u",NAHeap::bit2idx(k),NAHeap::computeTreeIndex(k<<8));
}