#ifndef HDA_H
#define HDA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* hda memory layout:
 * Note: if sizeof(size_t) == 4 and sizeof(user type) == 2
 * 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
 * .---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 * |   capacity    |     size      |  [0]  |  [1]  |  [2]  |  ...
 * '---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *                                 ^
 *                         pointer from user
 *
 * Simple example:
 *   int* array = NULL;
 *   hda_push(array, 34);
 *   hda_push(array, 69);
 *   hda_push(array, 42);
 *   for (size_t i = 0; i < hda_size(array); i++)
 *       printf("%i\n", array[i]);
 *   hda_free(array);
 * Output:
 *   34
 *   69
 *   42
 */

#ifndef HDA_INIT_CAP
#define HDA_INIT_CAP 64
#endif

#define hda_capacity(hda) (((size_t*)(void*)(hda))[-2])
#define hda_size(hda)     (((size_t*)(void*)(hda))[-1])

#define hda_capacity_s(hda) ((hda) ? hda_capacity(hda) : 0)
#define hda_size_s(hda)     ((hda) ? hda_size    (hda) : 0)

#define hda_reserve(hda, expect) do {                  \
    size_t* start = (void*)(hda);                      \
    size_t new_cap = hda_capacity_s(hda);              \
    if (new_cap >= (expect)) break;                    \
    if (new_cap == 0) new_cap = HDA_INIT_CAP;          \
    while (new_cap < (expect)) new_cap += new_cap / 2; \
    start = realloc(start - !!(hda) * 2,               \
        sizeof(size_t) * 2 + sizeof *(hda) * new_cap); \
    assert(start != NULL); start[0] = new_cap;         \
    if (!(hda)) start[1] = 0;                          \
    (hda) = (void*)(start + 2);                        \
} while (0)

#define hda_push(hda, value) do {          \
    hda_reserve(hda, hda_size_s(hda) + 1); \
    (hda)[hda_size(hda)++] = (value);      \
} while (0)

#define hda_push_many(hda, data, size) do {     \
    hda_reserve(hda, hda_size_s(hda) + (size)); \
    memmove((hda) + hda_size(hda), (data),      \
        (size) * sizeof *(hda));                \
    hda_size(hda) += (size);                    \
} while (0)

#define hda_free(hda) do free( \
    (size_t*)(void*)(hda) - !!(hda) * 2); while (0)

#endif /* HDA_H */