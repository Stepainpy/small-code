/* Simple analog of standard rand()
 * with using algorithm xoshiro256**
 */

#ifndef XOSHIRO_256_RAND_H
#define XOSHIRO_256_RAND_H

#include <stdint.h>

void xsr256seed(uint64_t seed);
uint64_t xsr256rand(void);
double xsr256randzo(void);

#endif /* XOSHIRO_256_RAND_H */

#ifdef XSR256RAND_IMPLEMENTATION

static uint64_t __xsr256_state[4]; /* single global state */

static inline uint64_t __xsr256_rotl(uint64_t n, int s) {
    return n << s | n >> (64 - s);
}

static uint64_t __xsr256_splitmix(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

void xsr256seed(uint64_t seed) {
    __xsr256_state[0] = __xsr256_splitmix(&seed);
    __xsr256_state[1] = __xsr256_splitmix(&seed);
    __xsr256_state[2] = __xsr256_splitmix(&seed);
    __xsr256_state[3] = __xsr256_splitmix(&seed);
}

uint64_t xsr256rand(void) {
    const uint64_t r = __xsr256_rotl(__xsr256_state[1] * 5, 7) * 9;
    const uint64_t t = __xsr256_state[1] << 17;

    __xsr256_state[2] ^= __xsr256_state[0];
    __xsr256_state[3] ^= __xsr256_state[1];
    __xsr256_state[1] ^= __xsr256_state[2];
    __xsr256_state[0] ^= __xsr256_state[3];

    __xsr256_state[2] ^= t;
    __xsr256_state[3] = __xsr256_rotl(__xsr256_state[3], 45);

    return r;
}

double xsr256randzo(void) {
    return (xsr256rand() >> 11) * 0x1.p-53;
}

#endif /* XSR256RAND_IMPLEMENTATION */