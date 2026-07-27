#ifndef PRNG_ADAPTOR_H
#define PRNG_ADAPTOR_H

#include <stdint.h>

typedef struct { uint32_t (*get)(void* ptr); void* ptr; } pa_engine_32_t;
typedef struct { uint64_t (*get)(void* ptr); void* ptr; } pa_engine_64_t;

/* Uniform distributions */

typedef struct { pa_engine_32_t engine; uint32_t min, max; } pa_uidistr_32_t;
typedef struct { pa_engine_64_t engine; uint64_t min, max; } pa_uidistr_64_t;

uint32_t pa_uidistr_32_get(pa_uidistr_32_t distr); /* [min, max] */
uint64_t pa_uidistr_64_get(pa_uidistr_64_t distr); /* [min, max] */

float pa_urdistr_32_get_cc(pa_engine_32_t engine); /* [0, 1] */
float pa_urdistr_32_get_co(pa_engine_32_t engine); /* [0, 1) */
float pa_urdistr_32_get_oo(pa_engine_32_t engine); /* (0, 1) */

double pa_urdistr_64_get_cc(pa_engine_64_t engine); /* [0, 1] */
double pa_urdistr_64_get_co(pa_engine_64_t engine); /* [0, 1) */
double pa_urdistr_64_get_oo(pa_engine_64_t engine); /* (0, 1) */

#endif /* PRNG_ADAPTOR_H */

#ifdef PRNG_ADAPTOR_IMPLEMENTATION

uint32_t pa_uidistr_32_get(pa_uidistr_32_t distr) {
    const uint32_t range = distr.max - distr.min + 1;

    uint64_t prod = (uint64_t)distr.engine.get(distr.engine.ptr) * (uint64_t)range;
    uint32_t lower = prod;

    if (lower < range) {
        const uint32_t threshold = -range % range;
        while (lower < threshold) {
            prod = (uint64_t)distr.engine.get(distr.engine.ptr) * (uint64_t)range;
            lower = prod;
        }
    }

    return (uint32_t)(prod >> 32) + distr.min;
}

typedef struct { uint64_t l, h; } uint128_t;

static uint128_t pai_mul128(uint64_t a, uint64_t b) {
    uint128_t out;
    uint64_t al, ah, bl, bh, albh, ahbl, old;

    al = a % (UINT64_C(1) << 32); ah = a >> 32;
    bl = b % (UINT64_C(1) << 32); bh = b >> 32;

    out.l = al * bl; out.h = ah * bh;
    albh  = al * bh; ahbl  = ah * bl;

    old = out.l; out.h += (out.l += albh << 32) < old; out.h += albh >> 32;
    old = out.l; out.h += (out.l += ahbl << 32) < old; out.h += ahbl >> 32;

    return out;
}

uint64_t pa_uidistr_64_get(pa_uidistr_64_t distr) {
    const uint64_t range = distr.max - distr.min + 1;

    uint128_t prod = pai_mul128(distr.engine.get(distr.engine.ptr), range);

    if (prod.l < range) {
        const uint64_t threshold = -range % range;
        while (prod.l < threshold)
            prod = pai_mul128(distr.engine.get(distr.engine.ptr), range);
    }

    return prod.h + distr.min;
}

float pa_urdistr_32_get_cc(pa_engine_32_t engine) { return engine.get(engine.ptr) * (1. / 4294967295.); }
float pa_urdistr_32_get_co(pa_engine_32_t engine) { return engine.get(engine.ptr) * (1. / 4294967296.); }
float pa_urdistr_32_get_oo(pa_engine_32_t engine) { return ((float)engine.get(engine.ptr) + 0.5) * (1. / 4294967296.); }

double pa_urdistr_64_get_cc(pa_engine_64_t engine) { return (engine.get(engine.ptr) >> 11) * (1. / 9007199254740991.); }
double pa_urdistr_64_get_co(pa_engine_64_t engine) { return (engine.get(engine.ptr) >> 11) * (1. / 9007199254740992.); }
double pa_urdistr_64_get_oo(pa_engine_64_t engine) { return ((double)(engine.get(engine.ptr) >> 12) + 0.5) * (1. / 4503599627370496.); }

#endif /* PRNG_ADAPTOR_IMPLEMENTATION */