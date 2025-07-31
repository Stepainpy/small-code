#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <limits.h>

#ifndef SHA256_DEF
#define SHA256_DEF
#endif

#define SHA256_HASH_BYTE_WIDTH 32

#if CHAR_BIT != 8
#error "Non 8-bit byte"
#endif

typedef unsigned char sha256_byte_t;

#if  ULONG_MAX == 0xFFFFFFFF
typedef unsigned long sha256_word_t;
#elif UINT_MAX == 0xFFFFFFFF
typedef unsigned int  sha256_word_t;
#else
#error "Not found 32-bit integer type"
#endif

typedef struct sha256_state_t {
    sha256_word_t h[8];
    sha256_word_t length_low;
    sha256_word_t length_high;
    sha256_byte_t input[64];
    sha256_byte_t uploaded;
} sha256_state_t;

typedef struct sha256_hash_t {
    sha256_byte_t bytes[32];
} sha256_hash_t;

#ifdef __cplusplus
extern "C" {
#endif

SHA256_DEF void sha256_begin(sha256_state_t* state);
SHA256_DEF void sha256_load (sha256_state_t* state, const void* data, size_t size);
SHA256_DEF void sha256_end  (sha256_state_t* state, sha256_hash_t* hash);

SHA256_DEF void sha256(const void* data, size_t size, sha256_hash_t* hash);

#ifdef __cplusplus
}
#endif

#endif /* SHA256_H */

#ifdef SHA256_IMPLEMENTATION

#include <string.h>

static sha256_word_t sha256_rotr(sha256_word_t n, int s) {
    return n >> s | n << (32 - s);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static sha256_word_t sha256_bswap(sha256_word_t n) {
    n = (n & 0xFFFF0000) >> 16 | (n & 0x0000FFFF) << 16;
    n = (n & 0xFF00FF00) >>  8 | (n & 0x00FF00FF) <<  8;
    return n;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define sha256_bswap
#else
#error "Unknown endianess"
#endif

static void sha256_round(sha256_state_t* s) {
    static const sha256_word_t sha256_k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    };
    sha256_word_t w[64]; size_t i;
    sha256_word_t a, b, c, d, e, f, g, h;
    sha256_word_t s0, s1, ch, ma, t1, t2;

    memcpy(w, s->input, sizeof s->input);
    memset(s->input, 0, sizeof s->input);
    s->uploaded = 0;

    for (i =  0; i < 16; i++) w[i] = sha256_bswap(w[i]);
    for (i = 16; i < 64; i++) {
        s0 = sha256_rotr(w[i - 15], 7) ^ sha256_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        s1 = sha256_rotr(w[i - 2], 17) ^ sha256_rotr(w[i - 2],  19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    a = s->h[0]; b = s->h[1]; c = s->h[2]; d = s->h[3];
    e = s->h[4]; f = s->h[5]; g = s->h[6]; h = s->h[7];

    for (i = 0; i < 64; i++) {
        s0 = sha256_rotr(a, 2) ^ sha256_rotr(a, 13) ^ sha256_rotr(a, 22);
        s1 = sha256_rotr(e, 6) ^ sha256_rotr(e, 11) ^ sha256_rotr(e, 25);
        ma = (a & b) ^ (a & c) ^ (b & c);
        ch = (e & f) ^ (~e & g);
        t1 = h + s1 + ch + sha256_k[i] + w[i];
        t2 = s0 + ma;

        h = g; g = f; f = e; e = t1 +  d;
        d = c; c = b; b = a; a = t1 + t2;
    }

    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d;
    s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

void sha256_begin(sha256_state_t* s) {
    if (!s) return;
    memset(s, 0, sizeof *s);
    s->h[0] = 0x6a09e667; s->h[1] = 0xbb67ae85;
    s->h[2] = 0x3c6ef372; s->h[3] = 0xa54ff53a;
    s->h[4] = 0x510e527f; s->h[5] = 0x9b05688c;
    s->h[6] = 0x1f83d9ab; s->h[7] = 0x5be0cd19;
}

void sha256_load(sha256_state_t* s, const void* data, size_t size) {
    size_t cap, min_size; sha256_word_t prev;
    if (!s || (!data && size > 0)) return;
    while (size > 0) {
        cap = sizeof s->input - s->uploaded;
        min_size = cap < size ? cap : size;
        memcpy(s->input + s->uploaded, data, min_size);

        data = (char*)data + min_size;
        s->uploaded += min_size;
        size -= min_size;

        prev = s->length_low;
        s->length_low += min_size;
        if (prev > s->length_low) ++s->length_high;

        if (s->uploaded >= sizeof s->input)
            sha256_round(s);
    }
}

void sha256_end(sha256_state_t* s, sha256_hash_t* h) {
    size_t i;
    if (!s || !h) return;

    s->input[s->uploaded++] = 0x80;
    if (s->uploaded > 56) sha256_round(s);

    s->length_high <<= 3;
    s->length_high |= s->length_low >> 29;
    s->length_low  <<= 3;
    ((sha256_word_t*)s->input)[14] = sha256_bswap(s->length_high);
    ((sha256_word_t*)s->input)[15] = sha256_bswap(s->length_low );
    sha256_round(s);

    for (i = 0; i < 8; i++) s->h[i] = sha256_bswap(s->h[i]);
    memcpy(h->bytes, s->h, sizeof h->bytes);
}

void sha256(const void* data, size_t size, sha256_hash_t* h) {
    sha256_state_t s;
    sha256_begin(&s);
    sha256_load(&s, data, size);
    sha256_end(&s, h);
}

#endif /* SHA256_IMPLEMENTATION */