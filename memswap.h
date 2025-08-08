#ifndef MEMORY_SWAP_H
#define MEMORY_SWAP_H

#include <string.h>

void memswap(void* first, void* second, size_t count);

#ifndef NO_SWAP_MACRO
#define swap(a, b) memswap(&(a), &(b), \
    sizeof(a) < sizeof(b) ? sizeof(a) : sizeof(b))
#endif

#endif /* MEMORY_SWAP_H */

#ifdef MEMSWAP_IMPLEMENTATION

void memswap(void* first, void* second, size_t count) {
    union {
        unsigned char hex[16];
        unsigned char octa[8];
        unsigned char four[4];
        unsigned char once[1];
    } t;
    unsigned char* fst = first;
    unsigned char* snd = second;

    while (count >= sizeof t.hex) {
        memcpy(t.hex, fst, sizeof t.hex);
        memcpy(fst,   snd, sizeof t.hex);
        memcpy(snd, t.hex, sizeof t.hex);
        count -= sizeof t.hex;
        fst   += sizeof t.hex;
        snd   += sizeof t.hex;
    }
    while (count >= sizeof t.octa) {
        memcpy(t.octa, fst, sizeof t.octa);
        memcpy(fst,    snd, sizeof t.octa);
        memcpy(snd, t.octa, sizeof t.octa);
        count -= sizeof t.octa;
        fst   += sizeof t.octa;
        snd   += sizeof t.octa;
    }
    while (count >= sizeof t.four) {
        memcpy(t.four, fst, sizeof t.four);
        memcpy(fst,    snd, sizeof t.four);
        memcpy(snd, t.four, sizeof t.four);
        count -= sizeof t.four;
        fst   += sizeof t.four;
        snd   += sizeof t.four;
    }
    while (count >= sizeof t.once) {
        memcpy(t.once, fst, sizeof t.once);
        memcpy(fst,    snd, sizeof t.once);
        memcpy(snd, t.once, sizeof t.once);
        count -= sizeof t.once;
        fst   += sizeof t.once;
        snd   += sizeof t.once;
    }
}

#endif /* MEMSWAP_IMPLEMENTATION */