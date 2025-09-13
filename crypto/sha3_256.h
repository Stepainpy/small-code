#ifndef SHA3_256_H
#define SHA3_256_H

#include <stddef.h>

#ifndef SHA3_256_DEF
#define SHA3_256_DEF
#endif

#define SHA3_256_HASH_BYTE_WIDTH 32

#ifndef UINT8_MAX
#  include <limits.h>
#  define UINT8_MAX 0xFF
#  if UCHAR_MAX == UINT8_MAX
typedef unsigned char uint8_t;
#  endif
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

typedef struct sha3_256_state_t {
    union {
        uint64_t d1  [25];
        uint64_t d2[5][5];
    } state;
    uint8_t input[136];
    uint8_t uploaded;
} sha3_256_state_t;

typedef struct sha3_256_hash_t {
    uint8_t bytes[32];
} sha3_256_hash_t;

#ifdef __cplusplus
extern "C" {
#endif

SHA3_256_DEF void sha3_256_begin(sha3_256_state_t* state);
SHA3_256_DEF void sha3_256_load (sha3_256_state_t* state, const void* data, size_t size);
SHA3_256_DEF void sha3_256_end  (sha3_256_state_t* state, sha3_256_hash_t* hash);

SHA3_256_DEF void sha3_256(const void* data, size_t size, sha3_256_hash_t* hash);

#ifdef __cplusplus
}
#endif

#endif /* SHA3_256_H */

#ifdef SHA3_256_IMPLEMENTATION

#include <string.h>

static uint64_t sha3_256_rotl(uint64_t n, int s) {
    return n << s | n >> (64 - s);
}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static uint64_t sha3_256_bswap(uint64_t n) {
    n = (n & 0xFFFFFFFF00000000ULL) >> 32 | (n & 0x00000000FFFFFFFFULL) << 32;
    n = (n & 0xFFFF0000FFFF0000ULL) >> 16 | (n & 0x0000FFFF0000FFFFULL) << 16;
    n = (n & 0xFF00FF00FF00FF00ULL) >>  8 | (n & 0x00FF00FF00FF00FFULL) <<  8;
    return n;
}
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define sha3_256_bswap(n) (n)
#else
#  error "Unknown endian"
#endif

static void sha3_256_round(sha3_256_state_t* s) {
    static const uint64_t rc[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL,
        0x800000000000808aULL, 0x8000000080008000ULL,
        0x000000000000808bULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL,
        0x000000000000008aULL, 0x0000000000000088ULL,
        0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL,
        0x8000000000008089ULL, 0x8000000000008003ULL,
        0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800aULL, 0x800000008000000aULL,
        0x8000000080008081ULL, 0x8000000000008080ULL,
        0x0000000080000001ULL, 0x8000000080008008ULL
    };

    static const int rotc[24] = {
         1,  3,  6, 10, 15, 21, 28, 36,
        45, 55,  2, 14, 27, 41, 56,  8,
        25, 43, 62, 18, 39, 61, 20, 44
    };

    static const int piln[24] = {
        10,  7, 11, 17, 18,  3,  5, 16,
         8, 21, 24,  4, 15, 23, 19, 13,
        12,  2, 20, 14, 22,  9,  6,  1
    };

    uint64_t tmp[5], t, *in = (uint64_t*)s->input;
    size_t ri, i, j;

    for (i = 0; i < sizeof s->input / sizeof(uint64_t); i++)
        s->state.d1[i] ^= sha3_256_bswap(in[i]);
    memset(s->input, 0, sizeof s->input);
    s->uploaded = 0;

    /* Loop part from: https://github.com/brainhub/SHA3IUF/blob/master/sha3.c */
    for (ri = 0; ri < 24; ri++) {
        for (i = 0; i < 5; i++)
            tmp[i] =
                s->state.d2[0][i] ^ s->state.d2[1][i] ^
                s->state.d2[2][i] ^ s->state.d2[3][i] ^
                s->state.d2[4][i];

        for (i = 0; i < 5; i++) {
            t = tmp[(i + 4) % 5] ^ sha3_256_rotl(tmp[(i + 1) % 5], 1);
            for (j = 0; j < 5; j++)
                s->state.d2[j][i] ^= t;
        }

        t = s->state.d1[1];
        for (i = 0; i < 24; i++) {
            j = piln[i];
            tmp[0] = s->state.d1[j];
            s->state.d1[j] = sha3_256_rotl(t, rotc[i]);
            t = tmp[0];
        }

        for (j = 0; j < 5; j++) {
            for (i = 0; i < 5; i++)
                tmp[i] = s->state.d2[j][i];
            for (i = 0; i < 5; i++)
                s->state.d2[j][i] ^= ~tmp[(i + 1) % 5] & tmp[(i + 2) % 5];
        }

        s->state.d1[0] ^= rc[ri];
    }
}

void sha3_256_begin(sha3_256_state_t* s) {
    if (s) memset(s, 0, sizeof *s);
}

void sha3_256_load(sha3_256_state_t* s, const void* data, size_t size) {
    size_t cap, min;
    if (!s || (!data && size > 0)) return;
    while (size > 0) {
        cap = sizeof s->input - s->uploaded;
        min = size < cap ? size : cap;
        memcpy(s->input + s->uploaded, data, min);

        data = (char*)data + min;
        s->uploaded += min;
        size -= min;

        if (s->uploaded >= sizeof s->input)
            sha3_256_round(s);
    }
}

void sha3_256_end(sha3_256_state_t* s, sha3_256_hash_t* h) {
    size_t i = 0;
    if (!s || !h) return;

    s->input[s->uploaded        ] |= 0x06;
    s->input[sizeof s->input - 1] |= 0x80;
    sha3_256_round(s);

    for (i = 0; i < 25; i++)
        s->state.d1[i] = sha3_256_bswap(s->state.d1[i]);
    memcpy(h->bytes, s->state.d1, sizeof h->bytes);
}

void sha3_256(const void* data, size_t size, sha3_256_hash_t* h) {
    sha3_256_state_t s;
    sha3_256_begin(&s);
    sha3_256_load(&s, data, size);
    sha3_256_end(&s, h);
}

#endif /* SHA3_256_IMPLEMENTATION */