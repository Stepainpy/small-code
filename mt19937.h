#ifndef MT19937_H
#define MT19937_H

#ifndef UINT32_MAX
#  include <limits.h>
#    if  UINT_MAX == 0xFFFFFFFF
typedef unsigned int  uint32_t;
#  elif ULONG_MAX == 0xFFFFFFFF
typedef unsigned long uint32_t;
#  endif
#endif

#ifndef MT19937_DEF
#define MT19937_DEF
#endif

typedef struct mt19937_t {
    uint32_t words[624];
    uint32_t index;
} mt19937_t;

#ifdef __cplusplus
extern "C" {
#endif

MT19937_DEF uint32_t mtrand(mt19937_t* mt);
MT19937_DEF void     mtseed(mt19937_t* mt, uint32_t seed);
MT19937_DEF void     mtskip(mt19937_t* mt, uint32_t skip);

#ifdef __cplusplus
}
#endif

#endif /* MT19937_H */

#ifdef MT19937_IMPLEMENTATION

static void mtstep(mt19937_t* mt) {
    uint32_t i, x;
    mt->index = 0;

    for (i = 0; i < 624; i++) {
        x = (mt->words[ i           ] & 0x80000000) |
            (mt->words[(i + 1) % 624] & 0x7FFFFFFF);
        mt->words[i] = mt->words[(i + 397) % 624] ^ (x >> 1);
        if (x & 1) mt->words[i] ^= 0x9908b0df;
    }
}

uint32_t mtrand(mt19937_t* mt) {
    uint32_t x;
    if (mt->index >= 624)
        mtstep(mt);

    x = mt->words[mt->index++];
    x ^= (x >> 11) & 0xFFFFFFFF;
    x ^= (x <<  7) & 0x9D2C5680;
    x ^= (x << 15) & 0xEFC60000;
    x ^= (x >> 18);

    return x;
}

void mtseed(mt19937_t* mt, uint32_t seed) {
    uint32_t i, x;
    mt->words[0] = seed;
    mt->index = 624;

    for (i = 1; i < 624; i++) {
        x = mt->words[i - 1];
        x ^= x >> 30;
        x *= 0x6c078965;
        x += i;
        mt->words[i] = x;
    }
}

void mtskip(mt19937_t* mt, uint32_t skip) {
    while (skip > 624 - mt->index) {
        skip -= 624 - mt->index;
        mtstep(mt);
    }
    mt->index += skip;
}

#endif /* MT19937_IMPLEMENTATION */