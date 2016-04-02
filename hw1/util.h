#ifndef __UTIL_H__
#define __UTIL_H__

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

#endif /* __UTIL_H__ */