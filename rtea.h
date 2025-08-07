#ifndef RTEA_ENCRYPTION_H
#define RTEA_ENCRYPTION_H

#ifndef UINT32_MAX
#  include <limits.h>
#    if  UINT_MAX == 0xFFFFFFFF
typedef unsigned int  uint32_t;
#  elif ULONG_MAX == 0xFFFFFFFF
typedef unsigned long uint32_t;
#  endif
#endif

typedef uint32_t rtea_block_t[2];
typedef uint32_t rtea_key4w_t[4];
typedef uint32_t rtea_key8w_t[8];

void rtea128en(rtea_block_t dst, const rtea_block_t src, const rtea_key4w_t key);
void rtea128de(rtea_block_t dst, const rtea_block_t src, const rtea_key4w_t key);

void rtea256en(rtea_block_t dst, const rtea_block_t src, const rtea_key8w_t key);
void rtea256de(rtea_block_t dst, const rtea_block_t src, const rtea_key8w_t key);

#endif /* RTEA_ENCRYPTION_H */

#ifdef RTEA_IMPLEMENTATION

void rtea128en(rtea_block_t dst, const rtea_block_t src, const rtea_key4w_t key) {
    int i = 0; dst[0] = src[0]; dst[1] = src[1];
    while (i < 48) {
        dst[1] += dst[0] + ((dst[0] << 6) ^ (dst[0] >> 8)) + key[i % 4] + i; i++;
        dst[0] += dst[1] + ((dst[1] << 6) ^ (dst[1] >> 8)) + key[i % 4] + i; i++;
    }
}

void rtea128de(rtea_block_t dst, const rtea_block_t src, const rtea_key4w_t key) {
    int i = 47; dst[0] = src[0]; dst[1] = src[1];
    while (i > -1) {
        dst[0] -= dst[1] + ((dst[1] << 6) ^ (dst[1] >> 8)) + key[i % 4] + i; i--;
        dst[1] -= dst[0] + ((dst[0] << 6) ^ (dst[0] >> 8)) + key[i % 4] + i; i--;
    }
}

void rtea256en(rtea_block_t dst, const rtea_block_t src, const rtea_key8w_t key) {
    int i = 0; dst[0] = src[0]; dst[1] = src[1];
    while (i < 64) {
        dst[1] += dst[0] + ((dst[0] << 6) ^ (dst[0] >> 8)) + key[i % 8] + i; i++;
        dst[0] += dst[1] + ((dst[1] << 6) ^ (dst[1] >> 8)) + key[i % 8] + i; i++;
    }
}

void rtea256de(rtea_block_t dst, const rtea_block_t src, const rtea_key8w_t key) {
    int i = 63; dst[0] = src[0]; dst[1] = src[1];
    while (i > -1) {
        dst[0] -= dst[1] + ((dst[1] << 6) ^ (dst[1] >> 8)) + key[i % 8] + i; i--;
        dst[1] -= dst[0] + ((dst[0] << 6) ^ (dst[0] >> 8)) + key[i % 8] + i; i--;
    }
}

#endif /* RTEA_IMPLEMENTATION */