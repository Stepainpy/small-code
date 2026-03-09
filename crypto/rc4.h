#ifndef RIVEST_CIPHER_4_H
#define RIVEST_CIPHER_4_H

#include <stddef.h>

typedef unsigned char rc4_byte_t;

typedef struct rc4_context_t {
    rc4_byte_t S[256], i, j;
} rc4_context_t;

rc4_context_t rc4_init(const void* key, size_t len);
rc4_byte_t rc4_get_byte(rc4_context_t* ctx);

#endif /* RIVEST_CIPHER_4_H */

#ifdef RC4_IMPLEMENTATION

rc4_context_t rc4_init(const void* key, size_t len) {
    const rc4_byte_t* K = key;
    rc4_context_t ctx = {0};
    int i, j, temp;

    for (i     = 0; i < 256; i++) ctx.S[i] = i;
    for (i = j = 0; i < 256; i++) {
        j = (j + ctx.S[i] + K[i % len]) & 255;
        temp     = ctx.S[i];
        ctx.S[i] = ctx.S[j];
        ctx.S[j] = temp    ;
    }

    return ctx;
}

rc4_byte_t rc4_get_byte(rc4_context_t* ctx) {
    rc4_byte_t temp;
    ctx->j += ctx->S[++ctx->i];
    temp           = ctx->S[ctx->i];
    ctx->S[ctx->i] = ctx->S[ctx->j];
    ctx->S[ctx->j] = temp          ;
    return ctx->S[(
        ctx->S[ctx->i] + ctx->S[ctx->j]
    ) & 255];
}

#endif /* RC4_IMPLEMENTATION */