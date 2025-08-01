#ifndef MT19937_H
#define MT19937_H

#include <limits.h>

#ifndef MT19937_DEF
#define MT19937_DEF
#endif

#if  ULONG_MAX == 0xFFFFFFFF
typedef unsigned long mt19937_word_t;
#elif UINT_MAX == 0xFFFFFFFF
typedef unsigned int  mt19937_word_t;
#else
#error "Not found 32-bit integer type"
#endif

typedef struct mt19937_t {
    mt19937_word_t words[624];
    mt19937_word_t index;
} mt19937_t;

#ifdef __cplusplus
extern "C" {
#endif

MT19937_DEF mt19937_word_t mt19937(mt19937_t* mt);
MT19937_DEF void mt19937_seed(mt19937_t* mt, mt19937_word_t seed);
MT19937_DEF void mt19937_skip(mt19937_t* mt, mt19937_word_t skip);

#ifdef __cplusplus
}
#endif

#endif /* MT19937_H */

#ifdef MT19937_IMPLEMENTATION

static void mt19937_round(mt19937_t* mt) {
    mt19937_word_t i, x;
    mt->index = 0;

    for (i = 0; i < 624; i++) {
        x = (mt->words[ i           ] & 0x80000000) |
            (mt->words[(i + 1) % 624] & 0x7FFFFFFF);
        mt->words[i] = mt->words[(i + 397) % 624] ^ (x >> 1);
        if (x & 1) mt->words[i] ^= 0x9908b0df;
    }
}

mt19937_word_t mt19937(mt19937_t* mt) {
    mt19937_word_t x;
    if (mt->index >= 624)
        mt19937_round(mt);

    x = mt->words[mt->index++];
    x ^= (x >> 11) & 0xFFFFFFFF;
    x ^= (x <<  7) & 0x9D2C5680;
    x ^= (x << 15) & 0xEFC60000;
    x ^= (x >> 18);

    return x;
}

void mt19937_seed(mt19937_t* mt, mt19937_word_t seed) {
    mt19937_word_t i, x;
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

void mt19937_skip(mt19937_t* mt, mt19937_word_t skip) {
    while (skip > 624 - mt->index) {
        skip -= 624 - mt->index;
        mt19937_round(mt);
    }
    mt->index += skip;
}

#endif /* MT19937_IMPLEMENTATION */