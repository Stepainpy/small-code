#include "rtea.h"
#include <string.h>

#define RTEAI_BLKSZ 8
#define RTEAI_BLKFILLER 0x01

#define rteai_xor(dst, lhs, rhs) do { \
    dst[0] = lhs[0] ^ rhs[0]; \
    dst[1] = lhs[1] ^ rhs[1]; \
} while (0)

#define rteai_xora(dst, rhs) rteai_xor(dst, dst, rhs)

#define rteai_inc(blk) do { if (!++blk[0]) ++blk[1]; } while (0)

typedef void (*rteai_onepass_t)(rtea_block_t, const rtea_block_t, const rtea_word_t*);

static int rteai_en_ecb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out; size_t got;
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        onepass(out, in, param.key);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en_cbc(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        rteai_xora(in, prev);
        onepass(out, in, param.key);
        memcpy(prev, out, RTEAI_BLKSZ);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en_cfb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        onepass(out, prev, param.key);
        rteai_xora(out, in);
        memcpy(prev, out, RTEAI_BLKSZ);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en_ofb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        onepass(out, prev, param.key);
        memcpy(prev, out, RTEAI_BLKSZ);
        rteai_xora(out, in);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en_ctr(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, ctr; size_t got;
    memcpy(ctr, param.init_vec, RTEAI_BLKSZ);
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        onepass(out, ctr, param.key);
        rteai_xora(out, in);
        rteai_inc(ctr);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en_pcbc(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    do {
        memset(in, RTEAI_BLKFILLER, RTEAI_BLKSZ);
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return 0;

        rteai_xora(prev, in);
        onepass(out, prev, param.key);
        rteai_xor(prev, in, out);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    } while (got == RTEAI_BLKSZ);
    return RTEAI_BLKSZ - got;
}

static int rteai_en(
    const rtea_io_t* io, rtea_param_t param,
    rtea_mode_t mode, rteai_onepass_t enfn, rteai_onepass_t defn
) { (void)defn;
    if (!param.key || !io || !io->rdfn || !io->wrfn)
        return RTEA_ERR_NULL_PTR;

    switch (mode) {
        case RTEA_MODE_ECB : return rteai_en_ecb (io, param, enfn);
        case RTEA_MODE_CBC : return rteai_en_cbc (io, param, enfn);
        case RTEA_MODE_CFB : return rteai_en_cfb (io, param, enfn);
        case RTEA_MODE_OFB : return rteai_en_ofb (io, param, enfn);
        case RTEA_MODE_CTR : return rteai_en_ctr (io, param, enfn);
        case RTEA_MODE_PCBC: return rteai_en_pcbc(io, param, enfn);
    }

    return RTEA_ERR_UNKNOWN_MODE;
}

static int rteai_de_ecb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out; size_t got;
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, in, param.key);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de_cbc(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, in, param.key);
        rteai_xora(out, prev);
        memcpy(prev, in, RTEAI_BLKSZ);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de_cfb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, prev, param.key);
        rteai_xora(out, in);
        memcpy(prev, in, RTEAI_BLKSZ);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de_ofb(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, prev, param.key);
        memcpy(prev, out, RTEAI_BLKSZ);
        rteai_xora(out, in);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de_ctr(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, ctr; size_t got;
    memcpy(ctr, param.init_vec, RTEAI_BLKSZ);
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, ctr, param.key);
        rteai_xora(out, in);
        rteai_inc(ctr);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de_pcbc(
    const rtea_io_t* io, rtea_param_t param,
    rteai_onepass_t onepass
) {
    rtea_block_t in, out, prev; size_t got;
    memcpy(prev, param.init_vec, RTEAI_BLKSZ);
    for (;;) {
        got = io->rdfn(in, 1, RTEAI_BLKSZ, io->rdctx);
        if (got == 0) return RTEA_ERR_OK;
        if (got < RTEAI_BLKSZ)
            return RTEA_ERR_NOT_FULLY_READ;

        onepass(out, in, param.key);
        rteai_xora(out, prev);
        rteai_xor(prev, in, out);

        if (!io->wrfn(out, RTEAI_BLKSZ, 1, io->wrctx))
            return RTEA_ERR_FAIL_WRITE;
    }
}

static int rteai_de(
    const rtea_io_t* io, rtea_param_t param,
    rtea_mode_t mode, rteai_onepass_t enfn, rteai_onepass_t defn
) {
    if (!param.key || !io || !io->rdfn || !io->wrfn)
        return RTEA_ERR_NULL_PTR;

    switch (mode) {
        case RTEA_MODE_ECB : return rteai_de_ecb (io, param, defn);
        case RTEA_MODE_CBC : return rteai_de_cbc (io, param, defn);
        case RTEA_MODE_CFB : return rteai_de_cfb (io, param, enfn);
        case RTEA_MODE_OFB : return rteai_de_ofb (io, param, enfn);
        case RTEA_MODE_CTR : return rteai_de_ctr (io, param, enfn);
        case RTEA_MODE_PCBC: return rteai_de_pcbc(io, param, defn);
    }

    return RTEA_ERR_UNKNOWN_MODE;
}

void rtea_128_onepass_en(
          rtea_block_t out,
    const rtea_block_t in,
    const rtea_word_t key[4]
) {
    rtea_word_t L, R; int i = 0;
    memcpy(&L, in    , 4);
    memcpy(&R, in + 1, 4);
    while (i < 48) {
        R += L + ((L << 6) ^ (L >> 8)) + key[i & 3] + i; i++;
        L += R + ((R << 6) ^ (R >> 8)) + key[i & 3] + i; i++;
    }
    memcpy(out    , &L, 4);
    memcpy(out + 1, &R, 4);
}

void rtea_128_onepass_de(
          rtea_block_t out,
    const rtea_block_t in,
    const rtea_word_t key[4]
) {
    rtea_word_t L, R; int i = 47;
    memcpy(&L, in    , 4);
    memcpy(&R, in + 1, 4);
    while (i >= 0) {
        L -= R + ((R << 6) ^ (R >> 8)) + key[i & 3] + i; i--;
        R -= L + ((L << 6) ^ (L >> 8)) + key[i & 3] + i; i--;
    }
    memcpy(out    , &L, 4);
    memcpy(out + 1, &R, 4);
}

void rtea_256_onepass_en(
          rtea_block_t out,
    const rtea_block_t in,
    const rtea_word_t key[8]
) {
    rtea_word_t L, R; int i = 0;
    memcpy(&L, in    , 4);
    memcpy(&R, in + 1, 4);
    while (i < 64) {
        R += L + ((L << 6) ^ (L >> 8)) + key[i & 7] + i; i++;
        L += R + ((R << 6) ^ (R >> 8)) + key[i & 7] + i; i++;
    }
    memcpy(out    , &L, 4);
    memcpy(out + 1, &R, 4);
}

void rtea_256_onepass_de(
          rtea_block_t out,
    const rtea_block_t in,
    const rtea_word_t key[8]
) {
    rtea_word_t L, R; int i = 63;
    memcpy(&L, in    , 4);
    memcpy(&R, in + 1, 4);
    while (i >= 0) {
        L -= R + ((R << 6) ^ (R >> 8)) + key[i & 7] + i; i--;
        R -= L + ((L << 6) ^ (L >> 8)) + key[i & 7] + i; i--;
    }
    memcpy(out    , &L, 4);
    memcpy(out + 1, &R, 4);
}

int rtea_128_en(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode)
    { return rteai_en(io, param, mode, rtea_128_onepass_en, rtea_128_onepass_de); }
int rtea_128_de(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode)
    { return rteai_de(io, param, mode, rtea_128_onepass_en, rtea_128_onepass_de); }
int rtea_256_en(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode)
    { return rteai_en(io, param, mode, rtea_256_onepass_en, rtea_256_onepass_de); }
int rtea_256_de(const rtea_io_t* io, rtea_param_t param, rtea_mode_t mode)
    { return rteai_de(io, param, mode, rtea_256_onepass_en, rtea_256_onepass_de); }