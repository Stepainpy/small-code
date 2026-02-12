#ifndef GALOIS_FIELD_2P8_H
#define GALOIS_FIELD_2P8_H

typedef unsigned char gf28_t;

int gf28_setup(gf28_t poly);
gf28_t gf28_poly(void);

gf28_t gf28_inv(gf28_t x);
gf28_t gf28_mul(gf28_t a, gf28_t b);
gf28_t gf28_div(gf28_t a, gf28_t b);
gf28_t gf28_pow(gf28_t a, gf28_t b);

gf28_t gf28_invp(gf28_t x,           gf28_t poly);
gf28_t gf28_mulp(gf28_t a, gf28_t b, gf28_t poly);
gf28_t gf28_divp(gf28_t a, gf28_t b, gf28_t poly);
gf28_t gf28_powp(gf28_t a, gf28_t b, gf28_t poly);

#endif /* GALOIS_FIELD_2P8_H */

#ifdef GF28_IMPLEMENTATION

static gf28_t gf28i_exp_tbl[256] = {0};
static gf28_t gf28i_log_tbl[256] = {0};

gf28_t gf28_mulp(gf28_t a, gf28_t b, gf28_t ip) {
    gf28_t c = 0;
    for (; a && b; b >>= 1) {
        c ^= a & -(b & 1);
        a = (a << 1) ^ (ip & -(a >> 7));
    }
    return c;
}

gf28_t gf28_powp(gf28_t a, gf28_t b, gf28_t ip) {
    gf28_t c = 1;
    for (; b; b >>= 1) {
        if (b & 1) c = gf28_mulp(c, a, ip);
        /*      */ a = gf28_mulp(a, a, ip);
    }
    return c;
}

gf28_t gf28_invp(gf28_t x, gf28_t ip) {
    return gf28_powp(x, 254, ip);
}

gf28_t gf28_divp(gf28_t a, gf28_t b, gf28_t ip) {
    return gf28_mulp(a, gf28_invp(b, ip), ip);
}

gf28_t gf28_mul(gf28_t a, gf28_t b) {
    unsigned x = gf28i_log_tbl[a] + gf28i_log_tbl[b];
    x = (x & 255) + (x >> 8);
    return a && b ? gf28i_exp_tbl[x] : 0;
}

gf28_t gf28_pow(gf28_t a, gf28_t b) {
    unsigned x = b * gf28i_log_tbl[a];
    x = (x & 255) + (x >> 8);
    x = (x & 255) + (x >> 8);
    return !b ? 1 : !a ? 0 : gf28i_exp_tbl[x];
}

gf28_t gf28_inv(gf28_t x) {
    return x ? gf28i_exp_tbl[255 - gf28i_log_tbl[x]] : 0;
}

gf28_t gf28_div(gf28_t a, gf28_t b) {
    return gf28_mul(a, gf28_inv(b));
}

int gf28_setup(gf28_t poly) {
    static const struct { gf28_t ip, pr; } polys[30] = {
        { 27, 3}, { 29, 2}, { 43, 2}, { 45, 2}, { 57, 3},
        { 63, 3}, { 77, 2}, { 95, 2}, { 99, 2}, {101, 2},
        {105, 2}, {113, 2}, {119, 3}, {123, 9}, {135, 2},
        {139, 6}, {141, 2}, {159, 3}, {163, 3}, {169, 2},
        {177, 6}, {189, 7}, {195, 2}, {207, 2}, {215, 7},
        {221, 6}, {231, 2}, {243, 6}, {245, 2}, {249, 3},
    };
    gf28_t i, x, prime;

    for (i = 0; i < 30; i++)
        if (poly == polys[i].ip) goto found;
    return 1;

found:
    gf28i_exp_tbl[255] = x = 1;
    gf28i_log_tbl[ 0 ] = poly;
    prime = polys[ i ].pr;
    for (i = 0; i < 255; i++) {
        gf28i_exp_tbl[i] = x;
        gf28i_log_tbl[x] = i;
        x = gf28_mulp(x, prime, poly);
    }
    return 0;
}

gf28_t gf28_poly(void) { return gf28i_log_tbl[0]; }

#endif /* GF28_IMPLEMENTATION */