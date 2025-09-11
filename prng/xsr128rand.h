/* Simple analog of standard rand()
 * with using algorithm xoshiro128**
 */

#ifndef XOSHIRO_128_RAND_H
#define XOSHIRO_128_RAND_H

#include <stdint.h>

void xsr128seed(uint32_t seed);
uint32_t xsr128rand(void);
float  xsr128randzo(void);

#endif /* XOSHIRO_128_RAND_H */

#ifdef XSR128RAND_IMPLEMENTATION

static uint32_t __xsr128_state[4]; /* single global state */

static inline uint32_t __xsr128_rotl(uint32_t n, int s) {
    return n << s | n >> (32 - s);
}

static uint32_t __xsr128_splitmix(uint32_t* x) {
    uint32_t z = (*x += 0x9e3779b9);
    z = (z ^ (z >> 15)) * 0x85ebca6b;
    z = (z ^ (z >> 13)) * 0xc2b2ae35;
    return z ^ (z >> 16);
}

void xsr128seed(uint32_t seed) {
    __xsr128_state[0] = __xsr128_splitmix(&seed);
    __xsr128_state[1] = __xsr128_splitmix(&seed);
    __xsr128_state[2] = __xsr128_splitmix(&seed);
    __xsr128_state[3] = __xsr128_splitmix(&seed);
}

uint32_t xsr128rand(void) {
    const uint32_t r = __xsr128_rotl(__xsr128_state[1] * 5, 7) * 9;
    const uint32_t t = __xsr128_state[1] << 9;

    __xsr128_state[2] ^= __xsr128_state[0];
    __xsr128_state[3] ^= __xsr128_state[1];
    __xsr128_state[1] ^= __xsr128_state[2];
    __xsr128_state[0] ^= __xsr128_state[3];

    __xsr128_state[2] ^= t;
    __xsr128_state[3] = __xsr128_rotl(__xsr128_state[3], 11);

    return r;
}

float xsr128randzo(void) {
    return (xsr128rand() >> 8) * 0x1.p-24f;
}

#endif /* XSR128RAND_IMPLEMENTATION */