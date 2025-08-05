#ifndef DA_H
#define DA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* da structure fields:
 * arr (type *) - pointer to array
 * len (size_t) - number of items
 * cap (size_t) - possible number of items
 */

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 64
#endif

#define da_reserve(da, newcap) do {     \
    if ((da)->cap >= (newcap)) break;   \
    if ((da)->cap == 0)                 \
        (da)->cap = DA_INIT_CAP;        \
    while ((da)->cap < (newcap))        \
        (da)->cap += (da)->cap / 2;     \
    (da)->arr = realloc((da)->arr,      \
        (da)->cap * sizeof *(da)->arr); \
    assert((da)->arr != NULL);          \
} while (0)

#define da_push(da, value) do {       \
    da_reserve(da, (da)->len + 1);    \
    (da)->arr[(da)->len++] = (value); \
} while (0)

#define da_push_many(da, data, size) do {  \
    da_reserve(da, (da)->len + (size));    \
    memmove((da)->arr + (da)->len, (data), \
        (size) * sizeof *(da)->arr);       \
    (da)->len += (size);                   \
} while (0)

#endif /* DA_H */