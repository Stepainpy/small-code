/* Irreducible polynomials (hex/dec) and first primitive element:
 * 11b  283  3 | 11d  285  2 | 12b  299  2 | 12d  301  2 | 139  313  3
 * 13f  319  3 | 14d  333  2 | 15f  351  2 | 163  355  2 | 165  357  2
 * 169  361  2 | 171  369  2 | 177  375  3 | 17b  379  9 | 187  391  2
 * 18b  395  6 | 18d  397  2 | 19f  415  3 | 1a3  419  3 | 1a9  425  2
 * 1b1  433  6 | 1bd  445  7 | 1c3  451  2 | 1cf  463  2 | 1d7  471  7
 * 1dd  477  6 | 1e7  487  2 | 1f3  499  6 | 1f5  501  2 | 1f9  505  3
 */

#ifndef GALOIS_FIELD_2P8_H
#define GALOIS_FIELD_2P8_H

#include <stdint.h>

#ifndef GF28_DFLT_IP
#define GF28_DFLT_IP 0x1b
#endif

#define gf28_mul(lhs, rhs) gf28_mul_ip((lhs), (rhs), GF28_DFLT_IP)
#define gf28_inv(mhs     ) gf28_inv_ip((mhs),        GF28_DFLT_IP)

uint8_t gf28_mul_ip(uint8_t lhs, uint8_t rhs, uint8_t ip);
uint8_t gf28_inv_ip(uint8_t mhs,              uint8_t ip);

typedef struct { uint8_t exp[256], log[256]; } gf28_eltbl_t;

void gf28_eltbl_init(gf28_eltbl_t* tbl, uint8_t ip, uint8_t prime);

uint8_t gf28_mul_el(uint8_t lhs, uint8_t rhs, const gf28_eltbl_t* tbl);
uint8_t gf28_inv_el(uint8_t mhs,              const gf28_eltbl_t* tbl);

#endif /* GALOIS_FIELD_2P8_H */

#ifdef GF28_IMPLEMENTATION

uint8_t gf28_mul_ip(uint8_t lhs, uint8_t rhs, uint8_t ip) {
    uint8_t mhs = 0;
    for (; lhs; lhs >>= 1) {
        if (lhs & 1) mhs ^= rhs;
        rhs = (rhs << 1) ^ (rhs & 0x80 ? ip : 0);
    }
    return mhs;
}

uint8_t gf28_inv_ip(uint8_t mhs, uint8_t ip) {
    uint8_t res = 1, pow = 254;
    for(; pow; pow >>= 1) {
        if (pow & 1) res = gf28_mul_ip(res, mhs, ip);
        mhs = gf28_mul_ip(mhs, mhs, ip);
    }
    return res;
}

void gf28_eltbl_init(gf28_eltbl_t* tbl, uint8_t ip, uint8_t prime) {
    uint8_t x = tbl->exp[255] = 1;
    for (size_t i = 0; i < 255; i++) {
        tbl->exp[i] = x;
        tbl->log[x] = i;
        x = gf28_mul_ip(x, prime, ip);
    }
}

uint8_t gf28_mul_el(uint8_t lhs, uint8_t rhs, const gf28_eltbl_t* tbl) {
    if (lhs == 0 || rhs == 0) return 0;
    unsigned mhs = tbl->log[lhs] + tbl->log[rhs];
    return tbl->exp[(mhs & 255) + (mhs >> 8)];
}

uint8_t gf28_inv_el(uint8_t mhs, const gf28_eltbl_t* tbl) {
    if (mhs == 0) return 0;
    return tbl->exp[255 - tbl->log[mhs]];
}

#endif /* GF28_IMPLEMENTATION */