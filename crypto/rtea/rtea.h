/* Encryption and decryption functions API
 *
 * rtea_io_t.rdfn like a fread
 * rtea_io_t.wrfn like a fwrite
 *
 * rtea_(128/256)_en returns padding byte count on success, negative value on failure
 * rtea_(128/256)_de returns 0 on success, negative value on failure
 */

#ifndef RTEA_BLOCK_CIPHER_H
#define RTEA_BLOCK_CIPHER_H

#include <limits.h>
#include <stddef.h>

#if UINT_MAX == 0xFFFFFFFFU
typedef unsigned rtea_word_t;
#elif ULONG_MAX == 0xFFFFFFFFUL
typedef unsigned long rtea_word_t;
#else
#  error Not detect 32-bit integer
#endif

typedef rtea_word_t rtea_block_t[2];

void rtea_128_onepass_en(rtea_block_t out, const rtea_block_t in, const rtea_word_t key[4]);
void rtea_128_onepass_de(rtea_block_t out, const rtea_block_t in, const rtea_word_t key[4]);

void rtea_256_onepass_en(rtea_block_t out, const rtea_block_t in, const rtea_word_t key[8]);
void rtea_256_onepass_de(rtea_block_t out, const rtea_block_t in, const rtea_word_t key[8]);

typedef enum {
    RTEA_MODE_ECB,
    RTEA_MODE_CBC,
    RTEA_MODE_CFB,
    RTEA_MODE_OFB,
    RTEA_MODE_CTR,
    RTEA_MODE_PCBC
} rtea_mode_t;

typedef enum {
    RTEA_ERR_NOT_FULLY_READ = -4,
    RTEA_ERR_UNKNOWN_MODE   = -3,
    RTEA_ERR_NULL_PTR       = -2,
    RTEA_ERR_FAIL_WRITE     = -1,
    RTEA_ERR_OK             =  0
} rtea_error_t;

typedef struct {
    size_t (*rdfn)(      void* dst, size_t size, size_t count, void* ctx);
    size_t (*wrfn)(const void* src, size_t size, size_t count, void* ctx);
    void *rdctx, *wrctx;
} rtea_io_t;

typedef struct {
    const void* key;
    const rtea_block_t init_vec;
} rtea_param_t;

int rtea_128_en(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode);
int rtea_128_de(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode);

int rtea_256_en(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode);
int rtea_256_de(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode);

#endif /* RTEA_BLOCK_CIPHER_H */