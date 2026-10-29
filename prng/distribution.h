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

/* U(0, 1) */ double distr_uniform_i0i1(distr_rand_t rand, void* state);
/* N(0, 1) */ double distr_normal_m0s1(distr_rand_t rand, void* state, double* second);

/* U(a, b) */ double distr_uniform(distr_uniform_t* param);
/* N(m, s) */ double distr_normal(distr_normal_t* param);

#endif /* RANDOM_DISTRIBUTION_H */

#ifdef DISTRIBUTION_IMPLEMENTATION

#include <math.h>

#ifndef NAN
#define NAN (0./0.)
#endif

double distr_uniform_i0i1(distr_rand_t rand, void* state) {
    return (double)(rand(state) >> 11) * (1. / 9007199254740991.);
}

double distr_normal_m0s1(distr_rand_t rand, void* state, double* second) {
    double x, y, s;
    do {
        x = 2. * distr_uniform_i0i1(rand, state) - 1.;
        y = 2. * distr_uniform_i0i1(rand, state) - 1.;
        s = x * x + y * y;
    } while (s > 1. || s == 0.);
    s = sqrt(-2. * log(s) / s);
    *second = x * s;
    return    y * s;
}

double distr_uniform(distr_uniform_t* param) {
    return param->a + (param->b - param->a) *
        distr_uniform_i0i1(param->rand, param->state);
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

#endif /* DISTRIBUTION_IMPLEMENTATION */