#ifndef RTEA_BLOCK_CIPHER_H
#define RTEA_BLOCK_CIPHER_H

void rtea128_block_encode(void* dest, const void* src, const void* key);
void rtea128_block_decode(void* dest, const void* src, const void* key);

void rtea256_block_encode(void* dest, const void* src, const void* key);
void rtea256_block_decode(void* dest, const void* src, const void* key);

#endif /* RTEA_BLOCK_CIPHER_H */

#ifdef RTEA_IMPLEMENTATION

#include <string.h>
#include <limits.h>

#   if  UINT_MAX == 0xFFFFFFFF
typedef unsigned int  rtea_word_t;
# elif ULONG_MAX == 0xFFFFFFFF
typedef unsigned long rtea_word_t;
# else
#error Not found 32-bit integer
#endif

void rtea128_block_encode(void* dest, const void* src, const void* key) {
    rtea_word_t L, R, K[4]; int i;
    memcpy(K, key, sizeof K);
    memcpy(&L, (const char*)src + 0, sizeof L);
    memcpy(&R, (const char*)src + 4, sizeof R);
    for (i = 0; i < 48;) {
        R += L + ((L << 6) ^ (L >> 8)) + K[i & 3] + i; i++;
        L += R + ((R << 6) ^ (R >> 8)) + K[i & 3] + i; i++;
    }
    memcpy((char*)dest + 0, &L, sizeof L);
    memcpy((char*)dest + 4, &R, sizeof R);
}

void rtea128_block_decode(void* dest, const void* src, const void* key) {
    rtea_word_t L, R, K[4]; int i;
    memcpy(K, key, sizeof K);
    memcpy(&L, (const char*)src + 0, sizeof L);
    memcpy(&R, (const char*)src + 4, sizeof R);
    for (i = 47; i >= 0;) {
        L -= R + ((R << 6) ^ (R >> 8)) + K[i & 3] + i; i--;
        R -= L + ((L << 6) ^ (L >> 8)) + K[i & 3] + i; i--;
    }
    memcpy((char*)dest + 0, &L, sizeof L);
    memcpy((char*)dest + 4, &R, sizeof R);
}

void rtea256_block_encode(void* dest, const void* src, const void* key) {
    rtea_word_t L, R, K[8]; int i;
    memcpy(K, key, sizeof K);
    memcpy(&L, (const char*)src + 0, sizeof L);
    memcpy(&R, (const char*)src + 4, sizeof R);
    for (i = 0; i < 64;) {
        R += L + ((L << 6) ^ (L >> 8)) + K[i & 7] + i; i++;
        L += R + ((R << 6) ^ (R >> 8)) + K[i & 7] + i; i++;
    }
    memcpy((char*)dest + 0, &L, sizeof L);
    memcpy((char*)dest + 4, &R, sizeof R);
}

void rtea256_block_decode(void* dest, const void* src, const void* key) {
    rtea_word_t L, R, K[8]; int i;
    memcpy(K, key, sizeof K);
    memcpy(&L, (const char*)src + 0, sizeof L);
    memcpy(&R, (const char*)src + 4, sizeof R);
    for (i = 63; i >= 0;) {
        L -= R + ((R << 6) ^ (R >> 8)) + K[i & 7] + i; i--;
        R -= L + ((L << 6) ^ (L >> 8)) + K[i & 7] + i; i--;
    }
    memcpy((char*)dest + 0, &L, sizeof L);
    memcpy((char*)dest + 4, &R, sizeof R);
}

#endif /* RTEA_IMPLEMENTATION */