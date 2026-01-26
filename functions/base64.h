#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

#define base64_ensize(len) (((len) / 3 + ((len) % 3 > 0)) * 4)
#define base64_desize(len) (((len) / 4 + ((len) % 4 > 1)) * 3)

void   base64_encode(char* dst, const void* src, size_t len);
size_t base64_decode(void* dst, const char* src);

#endif /* BASE64_H */

#ifdef BASE64_IMPLEMENTATION

#include <string.h>

#define BASE64I_DIGITS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

static unsigned long base64i_undigits(char digit) {
    if ('A' <= digit && digit <= 'Z') return digit - 'A';
    if ('a' <= digit && digit <= 'z') return digit - 'a' + 26;
    if ('0' <= digit && digit <= '9') return digit - '0' + 52;
    if ('+' == digit) return 62;
    if ('/' == digit) return 63;
    return -1L;
}

void base64_encode(char* dst, const void* src, size_t len) {
    const unsigned char* from = src; unsigned long buf = 0;
    for (; len > 0; len -= len > 2 ? 3 : len, dst += 4) {
        buf <<= 8; /* if (len > 0) */ buf |= *from++;
        buf <<= 8;    if (len > 1)    buf |= *from++;
        buf <<= 8;    if (len > 2)    buf |= *from++;
        dst[3] =    len < 3 ? '=' :    BASE64I_DIGITS[buf & 63]; buf >>= 6;
        dst[2] =    len < 2 ? '=' :    BASE64I_DIGITS[buf & 63]; buf >>= 6;
        dst[1] = /* len < 1 ? '=' : */ BASE64I_DIGITS[buf & 63]; buf >>= 6;
        dst[0] = /* len < 0 ? '=' : */ BASE64I_DIGITS[buf & 63]; buf >>= 6;
    }
}

size_t base64_decode(void* dst, const char* src) {
    unsigned char* to = dst; const char* end = src + strspn(src, BASE64I_DIGITS);
    unsigned long buf = 0; size_t len = 0; ptrdiff_t dist = 0;
    for (dist = end - src; dist > 1; dist = end - src, to += 3) {
        buf <<= 6; /* if (dist > 0) */ buf |= base64i_undigits(*src++);
        buf <<= 6; /* if (dist > 1) */ buf |= base64i_undigits(*src++);
        buf <<= 6;    if (dist > 2)    buf |= base64i_undigits(*src++);
        buf <<= 6;    if (dist > 3)    buf |= base64i_undigits(*src++);
        to[2] =    dist < 4 ? 0 :    buf & 255; buf >>= 8;
        to[1] =    dist < 3 ? 0 :    buf & 255; buf >>= 8;
        to[0] = /* dist < 2 ? 0 : */ buf & 255; buf >>= 8;
        len += dist > 3 ? 3 : dist - 1; dist = end - src;
    }
    return len;
}

#endif /* BASE64_IMPLEMENTATION */