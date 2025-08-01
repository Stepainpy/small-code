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
    mt19937_word_t k, y;
    mt->index = 0;

    for (k = 0; k < 624; k++) {
        y = (mt->words[ k           ] & 0x80000000) |
            (mt->words[(k + 1) % 624] & 0x7FFFFFFF);
        mt->words[k] = mt->words[(k + 397) % 624] ^ (y >> 1);
        if (y & 1) mt->words[k] ^= 0x9908b0df;
    }
}

mt19937_word_t mt19937(mt19937_t* mt) {
    mt19937_word_t z;
    if (mt->index >= 624)
        mt19937_round(mt);

    z = mt->words[mt->index++];
    z ^= (z >> 11) & 0xFFFFFFFF;
    z ^= (z <<  7) & 0x9D2C5680;
    z ^= (z << 15) & 0xEFC60000;
    z ^= (z >> 18);

    return z;
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