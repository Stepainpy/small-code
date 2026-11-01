#ifndef RANDOM_DISTRIBUTION_H
#define RANDOM_DISTRIBUTION_H

#include <stdint.h>

typedef uint64_t (*distr_rand_t)(void*);

typedef struct {
    distr_rand_t rand; void* state;
    double a, b; /* range [a; b] */
} distr_uniform_t;

typedef struct {
    distr_rand_t rand; void* state;
    double mean, stddev;
    double saved; /* init as NAN */
} distr_normal_t;

typedef struct {
    distr_rand_t rand; void* state;
    double* distr; size_t count;
} distr_discrete_t;

/* U[0, 1] */ double distr_uniform_i0i1(uint64_t random);
/* U[0, 1) */ double distr_uniform_i0e1(uint64_t random);
/* U(0, 1] */ double distr_uniform_e0i1(uint64_t random);
/* U(0, 1) */ double distr_uniform_e0e1(uint64_t random);
/* U[a, b] */ double distr_uniform(distr_uniform_t* param);

/* N(0, 1) */ double distr_normal_m0s1(distr_rand_t rand, void* state, double* second);
/* N(m, s) */ double distr_normal(distr_normal_t* param);

void distr_discrete_create(
    distr_discrete_t* param, double memory[],
    const double frequencies[], size_t count);
/* D{0, ..., n-1} */ uint64_t distr_discrete(distr_discrete_t* param);

#endif /* RANDOM_DISTRIBUTION_H */

#ifdef DISTRIBUTION_IMPLEMENTATION

#include <math.h>

#ifndef NAN
#define NAN (0./0.)
#endif

double distr_uniform_i0i1(uint64_t random) {
    return (double)(random >> 11) * (1. / 9007199254740991.);
}

double distr_uniform_i0e1(uint64_t random) {
    return (double)(random >> 11) * (1. / 9007199254740992.);
}

double distr_uniform_e0e1(uint64_t random) {
    return ((double)(random >> 12) + 0.5) * (1. / 4503599627370496.);
}

double distr_uniform_e0i1(uint64_t random) {
    return 1. - distr_uniform_i0e1(random);
}

double distr_normal_m0s1(distr_rand_t rand, void* state, double* second) {
    double x, y, s;
    do {
        x = 2. * distr_uniform_i0i1(rand(state)) - 1.;
        y = 2. * distr_uniform_i0i1(rand(state)) - 1.;
        s = x * x + y * y;
    } while (s > 1. || s == 0.);
    s = sqrt(-2. * log(s) / s);
    *second = x * s;
    return    y * s;
}

double distr_uniform(distr_uniform_t* param) {
    return param->a + (param->b - param->a) *
        distr_uniform_i0i1(param->rand(param->state));
}

double distr_normal(distr_normal_t* param) {
    double out;
    if (param->saved != param->saved)
        out = distr_normal_m0s1(
            param->rand, param->state, &param->saved);
    else {
        out = param->saved;
        param->saved = NAN;
    }
    return out * param->stddev + param->mean;
}

void distr_discrete_create(
    distr_discrete_t* param, double memory[],
    const double freqs[], size_t count
) {
    size_t i;

    memory[0] = freqs[0];
    for (i = 1; i < count; i++)
        memory[i] = memory[i - 1] + freqs[i];
    for (i = 0; i < count; i++)
        memory[i] /= memory[count - 1];

    param->distr = memory;
    param->count = count;
}

uint64_t distr_discrete(distr_discrete_t* param) {
    double r = distr_uniform_i0e1(param->rand(param->state));
    uint64_t i; for (i = 0; i < param->count; i++)
        if (r < param->distr[i]) return i;
    return 0;
}

#endif /* DISTRIBUTION_IMPLEMENTATION */