#ifndef THREE_VALUED_LOGIC_H
#define THREE_VALUED_LOGIC_H

typedef   signed char trit;  /* False/Unknown/True or -1/0/+1 */
typedef unsigned long tryte; /* 9 trits as F = 11, U = 00 and T = 01 */

#define tl1_false   ((trit)-1)
#define tl1_unknown ((trit) 0)
#define tl1_true    ((trit)+1)

trit tl1not(trit a);
trit tl1and(trit a, trit b);
trit tl1or (trit a, trit b);
trit tl1xor(trit a, trit b);

tryte tl9set(tryte a, trit b, unsigned i);
tryte tl9not(tryte a);
tryte tl9and(tryte a, tryte b);
tryte tl9or (tryte a, tryte b);
tryte tl9xor(tryte a, tryte b);

const char* tl9tostr(tryte a);

#endif /* THREE_VALUED_LOGIC_H */

#ifdef TVL_IMPLEMENTATION

trit tl1not(trit a        ) { return -a; }
trit tl1and(trit a, trit b) { return a < b ? a : b; }
trit tl1or (trit a, trit b) { return a > b ? a : b; }
trit tl1xor(trit a, trit b) { return tl1and(-tl1and(a, b), tl1or(a, b)); }

#define TLI_LBIT 0x15555ul

tryte tl9set(tryte a, trit b, unsigned i) {
    tryte mask = 3 << (i * 2), t = (tryte)b & 3;
    return i < 9 ? (a & ~mask) | t << (i * 2) : a;
}
tryte tl9not(tryte a) {
    return a ^ (a & TLI_LBIT) << 1;
}
tryte tl9and(tryte a, tryte b) {
    tryte has_false = ((a >> 1 & a) | (b >> 1 & b)) & TLI_LBIT;
    return has_false << 1 | ((a & b & TLI_LBIT) | has_false);
}
tryte tl9or(tryte a, tryte b) {
    return tl9not(tl9and(tl9not(a), tl9not(b)));
}
tryte tl9xor(tryte a, tryte b) {
    return tl9and(tl9or(a, b), tl9not(tl9and(a, b)));
}

const char* tl9tostr(tryte a) {
    static char buf[10] = {0}; int i;
    for (i = 9; i --> 0; a >>= 2)
        buf[i] = "UT-F"[a & 3];
    return buf;
}

#endif /* TVL_IMPLEMENTATION */