#include "quaternion.h"
#include <math.h>

/* Explain multiplication
 * (a + bi + cj + dk) *
 * (w + xi + yj + zk) =
 *
 * a w + a xi + a yj + a zk +
 * biw + bixi + biyj + bizk +
 * cjw + cjxi + cjyj + cjzk +
 * dkw + dkxi + dkyj + dkzk =
 *
 * aw  + axi + ayj + azk +
 * bwi - bx  + byk - bzj +
 * cwj - cxk - cy  + czi +
 * dwk + dxj - dyi - dz  =
 *
 * (aw - bx - cy - dz)  +
 * (ax + bw + cz - dy)i +
 * (ay - bz + cw + dx)j +
 * (az + by - cx + dw)k
 */

quaternion_t qadd(quaternion_t p, quaternion_t q) {
    return (quaternion_t){
        .a = p.a + q.a, .b = p.b + q.b,
        .c = p.c + q.c, .d = p.d + q.d
    };
}

quaternion_t qmul(quaternion_t p, quaternion_t q) {
    return (quaternion_t){
        .a = p.a*q.a - p.b*q.b - p.c*q.c - p.d*q.d,
        .b = p.a*q.b + p.b*q.a + p.c*q.d - p.d*q.c,
        .c = p.a*q.c - p.b*q.d + p.c*q.a + p.d*q.b,
        .d = p.a*q.d + p.b*q.c - p.c*q.b + p.d*q.a
    };
}

quaternion_t qmuld(quaternion_t q, double x) {
    return (quaternion_t){
        .a = q.a*x, .b = q.b*x,
        .c = q.c*x, .d = q.d*x
    };
}

quaternion_t qdivd(quaternion_t q, double x) {
    return (quaternion_t){
        .a = q.a/x, .b = q.b/x,
        .c = q.c/x, .d = q.d/x
    };
}

quaternion_t qconj(quaternion_t q) {
    return (quaternion_t){
        .a =  q.a, .b = -q.b,
        .c = -q.c, .d = -q.d
    };
}

quaternion_t qinv(quaternion_t q) {
    return qdivd(qconj(q), qnorm(q));
}

quaternion_t qunit(quaternion_t q) {
    return qdivd(q, qnorm(q));
}

double qnorm(quaternion_t q) {
    return sqrt(q.a*q.a + q.b*q.b + q.c*q.c + q.d*q.d);
}

void fputq(quaternion_t q, FILE* file) {
    fprintf(file, "%c%lg %c %lgi %c %lgj %c %lgk",
        q.a < 0. ? '-' : '+', fabs(q.a),
        q.b < 0. ? '-' : '+', fabs(q.b),
        q.c < 0. ? '-' : '+', fabs(q.c),
        q.d < 0. ? '-' : '+', fabs(q.d)
    );
}

void putq(quaternion_t q) {
    fputq(q, stdout); putchar('\n');
}

void q_near_zero_correct(quaternion_t* q) {
    q->a = fabs(q->a) < 1.E-16 ? 0. : q->a;
    q->b = fabs(q->b) < 1.E-16 ? 0. : q->b;
    q->c = fabs(q->c) < 1.E-16 ? 0. : q->c;
    q->d = fabs(q->d) < 1.E-16 ? 0. : q->d;
}