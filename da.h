#ifndef DA_H
#define DA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* da structure fields:
 * items    (type *) - pointer to array
 * count    (size_t) - number of items
 * capacity (size_t) - possible number of items
 */

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 64
#endif

#define da_reserve(da, newcap) do {             \
    if ((da)->capacity >= (newcap)) break;      \
    if ((da)->capacity == 0)                    \
        (da)->capacity = DA_INIT_CAP;           \
    while ((da)->capacity < (newcap))           \
        (da)->capacity += (da)->capacity / 2;   \
    (da)->items = realloc((da)->items,          \
        (da)->capacity * sizeof *(da)->items);  \
    assert((da)->items != NULL && "No memory"); \
} while (0)

#define da_push(da, value) do {           \
    da_reserve(da, (da)->count + 1);      \
    (da)->items[(da)->count++] = (value); \
} while (0)

#define da_push_many(da, data, size) do {     \
    da_reserve(da, (da)->count + (size));     \
    memcpy((da)->items + (da)->count, (data), \
        (size) * sizeof *(da)->items);        \
    (da)->count += (size);                    \
} while (0)

#endif /* DA_H */