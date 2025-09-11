#ifndef SWAP_H
#define SWAP_H

#include <string.h>

#define swap(a, b) do { \
    char swap[sizeof(a) == sizeof(b)  \
        ? sizeof(a) : (size_t)-1];    \
    memcpy(swap, &(a), sizeof(swap)); \
    memcpy(&(a), &(b), sizeof(swap)); \
    memcpy(&(b), swap, sizeof(swap)); \
} while (0)

#endif /* SWAP_H */