/* C Dynamic array
 * Set of function-macros for create and using dynamic arrays (further da).
 * For storage information about da use memory before data.
 *
 * Memory look like:
 *       === the direction of the ascending addresses ===>
 * ... | dtor | capacity | size | item 0 | item 1 | item 2 | ...
 *                              ^
 *                       pointer by data
 *
 * User code example:
 *   int* xs = NULL;
 *   cdarr_init(xs, NULL);
 *   cdarr_push_back(xs, 42);
 *   cdarr_push_back(xs, 69);
 *   assert(xs[0] == 42 && xs[1] == 69);
 *   cdarr_free(xs);
 *
 * TODO:
 * 1. documentation comments
 * 2. swap/copy
 */

#ifndef C_DYNAMIC_ARRAY_H
#define C_DYNAMIC_ARRAY_H

#include <string.h>

#ifndef CDARR_INIT_CAP
#define CDARR_INIT_CAP 16
#else
#  if CDARR_INIT_CAP < 2
#  error "The capacity value must be greater than one"
#  endif
#endif

#ifndef CDARR_REALLOC
#include <stdlib.h>
#define CDARR_REALLOC realloc
#endif

#ifndef CDARR_FREE
#include <stdlib.h>
#define CDARR_FREE free
#endif

#ifndef CDUTL_ASSERT
#include <assert.h>
#define CDUTL_ASSERT(expr, msg) assert((expr) && (msg))
#endif

typedef void (*cdutl_dtor_t)(void*);

#define CDARR_HEADER_SIZE (sizeof(size_t) * 2 + sizeof(cdutl_dtor_t))

/* Implementation detail */

#define cd__ptr_shift(ptr, offset) ((void*)((char*)(void*)(ptr) + (offset)))
#define cd__realloc_with_new_cap(arr, new_cap) do { \
    if (arr) (arr) = cd__ptr_shift(arr, -CDARR_HEADER_SIZE); \
    (arr) = CDARR_REALLOC((arr), CDARR_HEADER_SIZE + (new_cap) * sizeof *(arr)); \
    CDUTL_ASSERT((arr) != NULL, "Couldn't allocate memory"); \
    (arr) = cd__ptr_shift(arr, CDARR_HEADER_SIZE); \
    cdarr_capacity(arr) = (new_cap); \
} while (0)

/* Access to properties */

#define cdarr_size(arr)     (*(      size_t*)cd__ptr_shift(arr, -1 * sizeof(size_t)))
#define cdarr_capacity(arr) (*(      size_t*)cd__ptr_shift(arr, -2 * sizeof(size_t)))
#define cdarr_dtor(arr)     (*(cdutl_dtor_t*)cd__ptr_shift(arr, -CDARR_HEADER_SIZE ))

/* Creation/Destruction */

#define cdarr_init(arr, dtor_func) do { \
    cd__realloc_with_new_cap(arr, CDARR_INIT_CAP); \
    cdarr_dtor(arr) = (cdutl_dtor_t)(dtor_func); \
    cdarr_size(arr) = 0; \
} while (0)

#define cdarr_free(arr) do { \
    cdarr_clear(arr); \
    CDARR_FREE(cd__ptr_shift(arr, -CDARR_HEADER_SIZE)); \
    (arr) = NULL; \
} while (0)

/* Memory management */

#define cdarr_reserve(arr, expect) do { \
    size_t new_cap = cdarr_capacity(arr); \
    if (new_cap >= (expect)) break; \
    while (new_cap < (expect)) new_cap += new_cap / 2; \
    cd__realloc_with_new_cap(arr, new_cap); \
} while (0)

#define cdarr_shrink_to_fit(arr) do { \
    size_t new_cap = cdarr_size(arr); \
    new_cap = new_cap > 2 ? new_cap : 2; \
    cd__realloc_with_new_cap(arr, new_cap); \
} while (0)

/* Items management */

#define cdarr_push_back(arr, value) do { \
    cdarr_reserve(arr, cdarr_size(arr) + 1); \
    (arr)[cdarr_size(arr)++] = (value); \
} while (0)

#define cdarr_push_back_many(arr, values, count) do { \
    cdarr_reserve(arr, cdarr_size(arr) + (count)); \
    memmove(arr + cdarr_size(arr), (values), (count) * sizeof *(arr)); \
    cdarr_size(arr) += (count); \
} while (0)

#define cdarr_insert(arr, index, value) do { \
    CDUTL_ASSERT(0 <= (index) && (index) <= cdarr_size(arr), "Index out of bounds"); \
    cdarr_reserve(arr, cdarr_size(arr) + 1); \
    memmove((arr) + (index) + 1, (arr) + (index), \
        (cdarr_size(arr) - (index)) * sizeof *(arr)); \
    (arr)[(index)] = (value); ++cdarr_size(arr); \
} while (0)

#define cdarr_insert_many(arr, index, values, count) do { \
    CDUTL_ASSERT(0 <= (index) && (index) <= cdarr_size(arr), "Index out of bounds"); \
    cdarr_reserve(arr, cdarr_size(arr) + (count)); \
    memmove((arr) + (index) + (count), (arr) + (index), \
        (cdarr_size(arr) - (index)) * sizeof *(arr)); \
    memmove((arr) + (index), (values), (count) * sizeof *(arr)); \
    cdarr_size(arr) += (count); \
} while (0)

#define cdarr_pop_back(arr) do { \
    CDUTL_ASSERT(cdarr_size(arr) > 0, "Deleting from an empty array"); \
    if (cdarr_dtor(arr)) cdarr_dtor(arr)(arr + cdarr_size(arr) - 1); \
    --cdarr_size(arr); \
} while (0)

#define cdarr_pop_back_many(arr, count) do { size_t i; \
    CDUTL_ASSERT(cdarr_size(arr) >= (count), "Deleting more than is available"); \
    for (i = cdarr_size(arr) - (count); cdarr_dtor(arr) && i < cdarr_size(arr); i++) \
        cdarr_dtor(arr)(arr + i); \
    cdarr_size(arr) -= (count); \
} while (0)

#define cdarr_erase(arr, index) do { \
    CDUTL_ASSERT(0 <= (index) && (index) < cdarr_size(arr), "Index out of bounds"); \
    if (cdarr_dtor(arr)) cdarr_dtor(arr)(arr + (index)); \
    memmove((arr) + (index), (arr) + (index) + 1, \
        (cdarr_size(arr) - (index) - 1) * sizeof *(arr)); \
    --cdarr_size(arr); \
} while (0)

#define cdarr_erase_many(arr, index, count) do { \
    CDUTL_ASSERT(0 <= (index) && (index) < cdarr_size(arr), "Index out of bounds"); \
    CDUTL_ASSERT(cdarr_size(arr) - (index) >= (count), "Deleting more than is available"); \
    for (i = (index); cdarr_dtor(arr) && i < (index) + (count); i++) \
        cdarr_dtor(arr)(arr + i); \
    memmove((arr) + (index), (arr) + (index) + (count), \
        (cdarr_size(arr) - (index) - (count)) * sizeof *(arr)); \
    cdarr_size(arr) -= (count); \
} while (0)

#define cdarr_resize(arr, new_size, dflt_value) do { \
    /**/ if ((new_size) == cdarr_size(arr)) break; \
    else if ((new_size) > cdarr_size(arr)) { \
        size_t i; cdarr_reserve(arr, (new_size)); \
        for (i = cdarr_size(arr); i < (new_size); i++) \
            (arr)[i] = (dflt_value); \
    } else { \
        size_t i = (new_size); \
        for (; cdarr_dtor(arr) && i < cdarr_size(arr); i++) \
            cdarr_dtor(arr)(arr + i); \
    } \
    cdarr_size(arr) = (new_size); \
} while (0)

#define cdarr_clear(arr) do { size_t i = 0; \
    for (; cdarr_dtor(arr) && i < cdarr_size(arr); i++) \
        cdarr_dtor(arr)(arr + i); \
    cdarr_size(arr) = 0; \
} while (0)

#endif /* C_DYNAMIC_ARRAY_H */