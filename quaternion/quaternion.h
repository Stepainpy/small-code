#ifndef QUATERNION_H
#define QUATERNION_H

typedef struct quaternion_t {
    double a, b, c, d;
} quaternion_t;

#define QO ((quaternion_t){.a=1., .b=0., .c=0., .d=0.})
#define QI ((quaternion_t){.a=0., .b=1., .c=0., .d=0.})
#define QJ ((quaternion_t){.a=0., .b=0., .c=1., .d=0.})
#define QK ((quaternion_t){.a=0., .b=0., .c=0., .d=1.})

double       qnorm(quaternion_t q);
quaternion_t qconj(quaternion_t q);
quaternion_t qunit(quaternion_t q);
quaternion_t qinv (quaternion_t q);

quaternion_t qadd (quaternion_t p, quaternion_t q);
quaternion_t qmul (quaternion_t p, quaternion_t q);
quaternion_t qmuld(quaternion_t q, double x);
quaternion_t qdivd(quaternion_t q, double x);

void q_near_zero_correct(quaternion_t* q);

#include <stdio.h>

void fputq(quaternion_t q, FILE* file);
void  putq(quaternion_t q);

#endif /* QUATERNION_H */