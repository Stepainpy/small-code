/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                       C Dynamic array                       *
 * Set of function-macros for create and using dynamic arrays  *
 * (further da). For storage information about da use memory   *
 * before data.                                                *
 *                                                             *
 * Memory look like:                                           *
 *      === the direction of the ascending addresses ===>      *
 *  ... dtor | capacity | size | item 0 | item 1 | item 2 ...  *
 *                             ^                               *
 *                      pointer by data                        *
 *                                                             *
 * User code example:                                          *
 *   int* xs = NULL;                                           *
 *   cdarr_init(xs, NULL);                                     *
 *   cdarr_push_back(xs, 42);                                  *
 *   cdarr_push_back(xs, 69);                                  *
 *   assert(xs[0] == 42 && xs[1] == 69);                       *
 *   cdarr_free(xs);                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef C_DYNAMIC_ARRAY_H
#define C_DYNAMIC_ARRAY_H

#include <string.h>

/* The initial capacity after creating the array */
#ifndef CDARR_INIT_CAP
#  define CDARR_INIT_CAP 16
#else
#  if CDARR_INIT_CAP < 2
#    error "The capacity value must be greater than one"
#  endif
#endif

/* Fuctions for memory allocation */
#ifndef CDARR_CUSTOM_ALLOC
#  include <stdlib.h>
#  define CDARR_REALLOC realloc
#  define CDARR_FREE    free
#endif

/* Assert expression like standard 'assert'.
 * Expect two arguments, expression and message (string literal)
 */
#ifndef CDARR_ASSERT
#  include <assert.h>
#  define CDARR_ASSERT(expr, msg) assert((expr) && (msg))
#endif

/* Type of destructor for element type */
typedef void (*cdarr_dtor_t)(void*);

/* Implementation detail */

#define CDARR_HEADER_SIZE (sizeof(size_t) * 2 + sizeof(cdarr_dtor_t))
#define cd__ptr_shift(ptr, offset) ((void*)((char*)(void*)(ptr) + (offset)))
#define cd__realloc_with_new_cap(arr, new_cap, hdr_sz) do { \
    if (arr) (arr) = cd__ptr_shift(arr, -(hdr_sz)); \
    (arr) = CDARR_REALLOC((arr), (hdr_sz) + (new_cap) * sizeof *(arr)); \
    CDARR_ASSERT((arr) != NULL, "Couldn't allocate memory"); \
    (arr) = cd__ptr_shift(arr, hdr_sz); \
    cdarr_capacity(arr) = (new_cap); \
} while (0)

/* Access to properties
 * Return lvalue (avaible read and write)
 */

#define cdarr_size(arr)     (*(      size_t*)cd__ptr_shift(arr, -1 * sizeof(size_t)))
#define cdarr_capacity(arr) (*(      size_t*)cd__ptr_shift(arr, -2 * sizeof(size_t)))
#define cdarr_dtor(arr)     (*(cdarr_dtor_t*)cd__ptr_shift(arr, -CDARR_HEADER_SIZE ))

/* Access to elements */

#ifndef CDARR_ENABLE_SIZE_ASSERT
#  define cdarr_first(arr) ((arr)[         0         ])
#  define cdarr_last(arr)  ((arr)[cdarr_size(arr) - 1])
#else
#  define cdarr_first(arr) (CDARR_ASSERT(cdarr_size(arr) > 0, \
    "There are no elements in the array"), (arr)[         0         ])
#  define cdarr_last(arr)  (CDARR_ASSERT(cdarr_size(arr) > 0, \
    "There are no elements in the array"), (arr)[cdarr_size(arr) - 1])
#endif

/* Iterators */

#define cdarr_begin(arr) (arr)
#define cdarr_end(arr)  ((arr) + cdarr_size(arr))

/* Declare for-loop with variable 'iter'  */
#define cdarr_foreach(arr, iter) \
for (iter = cdarr_begin(arr); iter < cdarr_end(arr); iter++)

/* Creation/Destruction */

/* Initializes the array from NULL and pointer to destructor */
#define cdarr_init(arr, dtor_func) do { \
    cd__realloc_with_new_cap(arr, CDARR_INIT_CAP, CDARR_HEADER_SIZE); \
    cdarr_dtor(arr) = (cdarr_dtor_t)(dtor_func); \
    cdarr_size(arr) = 0; \
} while (0)

/* Frees up the memory allocated for the array */
#define cdarr_free(arr) do { \
    cdarr_clear(arr); \
    CDARR_FREE(cd__ptr_shift(arr, -CDARR_HEADER_SIZE)); \
    (arr) = NULL; \
} while (0)

/* Create copy 'dst' from 'src'. 'dst' and 'src' is cdarray */
#define cdarr_copy(dst, src) do { \
    if (dst) cdarr_free(dst); \
    cdarr_init(dst, cdarr_dtor(src)); \
    cdarr_push_back_many(dst, src, cdarr_size(src)); \
} while (0)

/* Swapping two arrays (pointers) */
#define cdarr_swap(fst, snd) do { \
    void* cd__t; \
    cd__t = (fst); \
    (fst) = (snd); \
    (snd) = cd__t; \
} while (0)

/* Memory management */

/* Reserves memory for at least 'expect' items */
#define cdarr_reserve(arr, expect) do { \
    size_t cd__newcap = cdarr_capacity(arr); \
    if (cd__newcap >= (expect)) break; \
    while (cd__newcap < (expect)) \
        cd__newcap += cd__newcap / 2; \
    cd__realloc_with_new_cap(arr, cd__newcap, CDARR_HEADER_SIZE); \
} while (0)

/* Frees up the free capacity */
#define cdarr_shrink_to_fit(arr) do { \
    size_t cd__newcap = cdarr_size(arr); \
    cd__newcap = cd__newcap > 2 ? cd__newcap : 2; \
    cd__realloc_with_new_cap(arr, cd__newcap, CDARR_HEADER_SIZE); \
} while (0)

/* Items management */

/* Adds a new element to the end of the array */
#define cdarr_push_back(arr, value) do { \
    cdarr_reserve(arr, cdarr_size(arr) + 1); \
    (arr)[cdarr_size(arr)++] = (value); \
} while (0)

/* Adds a new elements to the end of the array from pointer 'values' */
#define cdarr_push_back_many(arr, values, count) do { \
    cdarr_reserve(arr, cdarr_size(arr) + (count)); \
    memmove(arr + cdarr_size(arr), (values), (count) * sizeof *(arr)); \
    cdarr_size(arr) += (count); \
} while (0)

/* Adds a new element to the array at the location of the 'index'.
 * If 'index' equal size of array, then equal cdarr_push_back
 */
#define cdarr_insert(arr, index, value) do { \
    CDARR_ASSERT(0 <= (index) && (index) <= cdarr_size(arr), "Index out of bounds"); \
    cdarr_reserve(arr, cdarr_size(arr) + 1); \
    memmove((arr) + (index) + 1, (arr) + (index), \
        (cdarr_size(arr) - (index)) * sizeof *(arr)); \
    (arr)[(index)] = (value); ++cdarr_size(arr); \
} while (0)

/* Adds a new elements to the array at the location of the 'index' from pointer 'values'.
 * If 'index' equal size of array, then equal cdarr_push_back
 */
#define cdarr_insert_many(arr, index, values, count) do { \
    CDARR_ASSERT(0 <= (index) && (index) <= cdarr_size(arr), "Index out of bounds"); \
    cdarr_reserve(arr, cdarr_size(arr) + (count)); \
    memmove((arr) + (index) + (count), (arr) + (index), \
        (cdarr_size(arr) - (index)) * sizeof *(arr)); \
    memmove((arr) + (index), (values), (count) * sizeof *(arr)); \
    cdarr_size(arr) += (count); \
} while (0)

/* Deletes the last element of the array */
#define cdarr_pop_back(arr) do { \
    CDARR_ASSERT(cdarr_size(arr) > 0, "Deleting from an empty array"); \
    if (cdarr_dtor(arr)) cdarr_dtor(arr)(arr + cdarr_size(arr) - 1); \
    --cdarr_size(arr); \
} while (0)

/* Deletes the last elements of the array with 'count' */
#define cdarr_pop_back_many(arr, count) do { size_t cd__i; \
    CDARR_ASSERT(cdarr_size(arr) >= (count), "Deleting more than is available"); \
    for (cd__i = cdarr_size(arr) - (count); \
        cdarr_dtor(arr) && cd__i < cdarr_size(arr); cd__i++) \
        cdarr_dtor(arr)(arr + cd__i); \
    cdarr_size(arr) -= (count); \
} while (0)

/* Deletes an array element by 'index' */
#define cdarr_erase(arr, index) do { \
    CDARR_ASSERT(0 <= (index) && (index) < cdarr_size(arr), "Index out of bounds"); \
    if (cdarr_dtor(arr)) cdarr_dtor(arr)(arr + (index)); \
    memmove((arr) + (index), (arr) + (index) + 1, \
        (cdarr_size(arr) - (index) - 1) * sizeof *(arr)); \
    --cdarr_size(arr); \
} while (0)

/* Deletes an array elements by 'index' */
#define cdarr_erase_many(arr, index, count) do { size_t cd__i; \
    CDARR_ASSERT(0 <= (index) && (index) < cdarr_size(arr), "Index out of bounds"); \
    CDARR_ASSERT(cdarr_size(arr) - (index) >= (count), "Deleting more than is available"); \
    for (cd__i = (index); cdarr_dtor(arr) && cd__i < (index) + (count); cd__i++) \
        cdarr_dtor(arr)(arr + cd__i); \
    memmove((arr) + (index), (arr) + (index) + (count), \
        (cdarr_size(arr) - (index) - (count)) * sizeof *(arr)); \
    cdarr_size(arr) -= (count); \
} while (0)

/* Set new size for array.
 * If old size = new size, then do nothing
 * If old size > new size, then like cdarr_pop_back_many(old - new)
 * If old size < new size, then use 'dflt_value' for new elements
 */
#define cdarr_resize(arr, new_size, dflt_value) do { \
    if ((new_size) == cdarr_size(arr)) break; \
    else if ((new_size) > cdarr_size(arr)) { \
        size_t cd__i; cdarr_reserve(arr, (new_size)); \
        for (cd__i = cdarr_size(arr); cd__i < (new_size); cd__i++) \
            (arr)[cd__i] = (dflt_value); \
    } else { \
        size_t cd__i = (new_size); \
        for (; cdarr_dtor(arr) && cd__i < cdarr_size(arr); cd__i++) \
            cdarr_dtor(arr)(arr + cd__i); \
    } \
    cdarr_size(arr) = (new_size); \
} while (0)

/* Deletes all elements from array */
#define cdarr_clear(arr) do { size_t cd__i = 0; \
    for (; cdarr_dtor(arr) && cd__i < cdarr_size(arr); cd__i++) \
        cdarr_dtor(arr)(arr + cd__i); \
    cdarr_size(arr) = 0; \
} while (0)

#endif /* C_DYNAMIC_ARRAY_H */