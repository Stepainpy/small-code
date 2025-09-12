#ifndef MT19937_64_H
#define MT19937_64_H

#ifndef MT19937_64_DEF
#define MT19937_64_DEF
#endif

#ifndef UINT64_MAX
#  include <limits.h>
#  define UINT64_MAX 0xFFFFFFFFFFFFFFFFULL
#    if      ULONG_MAX == UINT64_MAX
typedef unsigned long      uint64_t;
#  elif ULONG_LONG_MAX == UINT64_MAX
typedef unsigned long long uint64_t;
#  endif
#endif

typedef struct mt19937_64_t {
    uint64_t words[312];
    uint64_t index;
} mt19937_64_t;

#ifdef __cplusplus
extern "C" {
#endif

MT19937_64_DEF uint64_t mt64rand(mt19937_64_t* mt);
MT19937_64_DEF void     mt64seed(mt19937_64_t* mt, uint64_t seed);
MT19937_64_DEF void     mt64skip(mt19937_64_t* mt, uint64_t skip);

#ifdef __cplusplus
}
#endif

#endif /* MT19937_64_H */

#ifdef MT19937_64_IMPLEMENTATION

static void mt64step(mt19937_64_t* mt) {
    uint64_t i, x;
    mt->index = 0;

    for (i = 0; i < 312; i++) {
        x = (mt->words[ i           ] & 0xFFFFFFFF80000000ULL) |
            (mt->words[(i + 1) % 312] & 0x000000007FFFFFFFULL);
        mt->words[i] = mt->words[(i + 156) % 312] ^ (x >> 1);
        if (x & 1) mt->words[i] ^= 0xb5026f5aa96619e9ULL;
    }
}

uint64_t mt64rand(mt19937_64_t* mt) {
    uint64_t x;
    if (mt->index >= 312)
        mt64step(mt);

    x = mt->words[mt->index++];
    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71d67fffeda60000ULL;
    x ^= (x << 37) & 0xfff7eee000000000ULL;
    x ^= (x >> 43);

    return x;
}

void mt64seed(mt19937_64_t* mt, uint64_t seed) {
    uint64_t i, x;
    mt->words[0] = seed;
    mt->index = 312;

    for (i = 1; i < 312; i++) {
        x = mt->words[i - 1];
        x ^= x >> 62;
        x *= 0x5851f42d4c957f2dULL;
        x += i;
        mt->words[i] = x;
    }
}

void mt64skip(mt19937_64_t* mt, uint64_t skip) {
    while (skip > 312 - mt->index) {
        skip -= 312 - mt->index;
        mt64step(mt);
    }
    mt->index += skip;
}

#endif /* MT19937_64_IMPLEMENTATION */