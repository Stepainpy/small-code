#ifndef BSEARCH_FIND_PLACE_H
#define BSEARCH_FIND_PLACE_H

#include <stddef.h>

/* Return index for insert in sorted array.
 * If returned index equal size of array, then add item to end.
 */
size_t bsearch_find_index(
    const void* key, const void* data, size_t count,
    size_t size, int (*cmp)(const void*, const void*)
);

#endif /* BSEARCH_FIND_PLACE_H */

#ifdef BS_INDEX_IMPLEMENTATION

size_t bsearch_find_index(
    const void* key, const void* data, size_t count,
    size_t size, int (*cmp)(const void*, const void*)
) {
    size_t beg, end, mid; int c;
    const char* ptr = data;
    beg = 0; end = count;

    while (end - beg > 1) {
        mid = beg + (end - beg) / 2;
        c = cmp(key, ptr + size * mid);
        if (c < 0) end = mid;
        else       beg = mid;
    }

    if (end - beg == 0) return beg;
    c = cmp(key, ptr + size * beg);
    return beg + (c > 0);
}

#endif /* BS_INDEX_IMPLEMENTATION */