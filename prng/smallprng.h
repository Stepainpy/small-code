#ifndef SMALL_STATE_PRNG_H
#define SMALL_STATE_PRNG_H

#include <stdint.h>

uint32_t splitmix32(uint32_t* state);
uint32_t mulberry32(uint32_t* state);

#endif /* SMALL_STATE_PRNG_H */

#ifdef SMALLPRNG_IMPLEMENTATION

uint32_t splitmix32(uint32_t* state) {
    uint32_t x = (*state += 0x9e3779b9);
    x = (x ^ x >> 15) * 0x85ebca6b;
    x = (x ^ x >> 13) * 0xc2b2ae35;
    return x ^ x >> 16;
}

uint32_t mulberry32(uint32_t* state) {
    uint32_t x = (*state += 0x6d2b79f5);
    x  =     (x ^ x >> 15) * (x | 1);
    x ^= x + (x ^ x >>  7) * (x | 1);
    return x ^ x >> 14;
}

#endif /* SMALLPRNG_IMPLEMENTATION */