#ifndef __UTIL_H__
#define __UTIL_H__

#include <stddef.h>

#define bit(x) (1ll << (x))
#define get_bits(x, l, r) (((x) >> (l)) & (bit((r) - (l)) - 1))

static inline int min(int a, int b) {
    return a < b ? a : b;
}

static inline int max(int a, int b) {
    return a > b ? a : b;
}

static inline uint64_t align_up(uint64_t val, uint64_t al) {
    if (val % al) {
        val += al - val%al;
    }
    return val;
}

static inline void barrier() {
    __asm__ volatile ("" : : : "memory");
}

static inline size_t strlen(const char * str) {
    size_t res = 0;
    while (str[res] != 0)  {
        ++res;
    }
    return res;
}

static inline int strncmp (const char * from, const char * to, size_t len) {
    unsigned int i = 0;
    for (;from[i] == to[i] && from[i] != 0 && i < len; ++i){
    }
    if (from[i] == 0 && to[i] == 0) {
        return 0;
    }
    if (i == len) {
        return 0;
    }
    if (from[i] < to[i]) {
        return -1;
    } else if (from[i] == to[i]) {
        return 0;
    } else {
        return 1;
    }

}

#endif /* __UTIL_H__ */