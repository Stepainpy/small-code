#ifndef SEQUENCE_ALGORITHMS_H
#define SEQUENCE_ALGORITHMS_H

#include <stddef.h>

#if __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
typedef unsigned char bool;
#  define false ((bool)0)
#  define true  ((bool)1)
#endif

#define ptr_advance(ptr, size, count) \
    ((void*)((char*)(ptr) + (size) * (count)))

#define ptr_distance(ptr1, ptr2, size) \
    (((char*)(ptr1) - (char*)(ptr2)) / (size))

void memswap(void* lhs, void* rhs, size_t size);

void reverse(void* base, size_t count, size_t size);

bool next_permutation(void* base, size_t count, size_t size, bool (*oper)(const void*, const void*));
bool prev_permutation(void* base, size_t count, size_t size, bool (*oper)(const void*, const void*));

#endif /* SEQUENCE_ALGORITHMS_H */

#ifdef SEQALGO_IMPLEMENTATION

void memswap(void* lhs, void* rhs, size_t size) {
    unsigned char* l = lhs;
    unsigned char* r = rhs;
    if (!lhs || !rhs) return;
    for (; size --> 0; ++l, ++r) {
        unsigned char t = *l; *l = *r; *r = t;
    }
}

void reverse(void* base, size_t count, size_t size) {
    size_t i = 0, j = count - 1;
    if (!base || !count || !size) return;
    for (; i < j; ++i, --j)
        memswap(ptr_advance(base, size, i),
                ptr_advance(base, size, j), size);
}

bool next_permutation(void* base, size_t count, size_t size, bool (*oper)(const void*, const void*)) {
    void* i;
    if (!base || !size || !oper) return false;
    if (count < 2) return false;

    i = ptr_advance(base, size, count - 1);
    while (true) {
        void* ii = i;
        i = ptr_advance(i, size, -1);
        if (oper(i, ii)) {
            void* j = ptr_advance(base, size, count);
        repeate:
            j = ptr_advance(j, size, -1);
            if (!oper(i, j)) goto repeate;
            memswap(i, j, size);
            reverse(ii, count - ptr_distance(ii, base, size), size);
            return true;
        }
        if (i == base) {
            reverse(base, count, size);
            return false;
        }
    }
}

bool prev_permutation(void* base, size_t count, size_t size, bool (*oper)(const void*, const void*)) {
    void* i;
    if (!base || !size || !oper) return false;
    if (count < 2) return false;

    i = ptr_advance(base, size, count - 1);
    while (true) {
        void* ii = i;
        i = ptr_advance(i, size, -1);
        if (oper(ii, i)) {
            void* j = ptr_advance(base, size, count);
        repeate:
            j = ptr_advance(j, size, -1);
            if (!oper(j, i)) goto repeate;
            memswap(i, j, size);
            reverse(ii, count - ptr_distance(ii, base, size), size);
            return true;
        }
        if (i == base) {
            reverse(base, count, size);
            return false;
        }
    }
}

#endif /* SEQALGO_IMPLEMENTATION */