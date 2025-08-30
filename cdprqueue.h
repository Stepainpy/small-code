/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * C Dynamic priority queue. Adaptor for cdarray.    *
 *                                                   *
 * Memory look like:                                 *
 * === the direction of the ascending addresses ===> *
 *   ... cmp | dtor | capacity | size | item 0 ...   *
 *                                    ^              *
 *                             pointer by data       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef C_DYNAMIC_PRIORITY_QUEUE_H
#define C_DYNAMIC_PRIORITY_QUEUE_H

#include "cdarray.h"

/* Type of comparison function for element type
 * Is three-way comparison and if
 * 2 < 5 => cmp(2, 5) < 0
 * 3 = 3 => cmp(3, 3) = 0
 * 5 > 2 => cmp(5, 2) > 0
 * then be max-heap (from big to small)
 */
typedef int (*cdprq_cmp_t)(const void*, const void*);

/* Implementation detail */

#define CDPRQ_HEADER_SIZE (CDARR_HEADER_SIZE + sizeof(cdprq_cmp_t))
#define cd__swap_by_ptr(fst, snd) do { \
    char cd__buf[sizeof *(fst)]; \
    memmove(cd__buf, (fst), sizeof cd__buf); \
    memmove((fst),   (snd), sizeof cd__buf); \
    memmove((snd), cd__buf, sizeof cd__buf); \
} while (0)
#define cd__heapify(prq, index) do { \
    size_t cd__b, cd__l, cd__r, cd__i = (index); \
    while (1) { \
        cd__b = cd__i; \
        cd__l = cd__i * 2 + 1; \
        cd__r = cd__i * 2 + 2; \
        if (cd__l < cdarr_size(prq) && \
            cdprq_cmp(prq)((prq) + cd__l, (prq) + cd__b) > 0) cd__b = cd__l; \
        if (cd__r < cdarr_size(prq) && \
            cdprq_cmp(prq)((prq) + cd__r, (prq) + cd__b) > 0) cd__b = cd__r; \
        if (cd__b == cd__i) break; \
        cd__swap_by_ptr((prq) + cd__i, (prq) + cd__b); \
        cd__i = cd__b; \
    } \
} while (0)

/* Access to properties
 * Return lvalue (avaible read and write)
 */

#define cdprq_cmp(prq) (*(cdprq_cmp_t*)cd__ptr_shift(prq, -CDPRQ_HEADER_SIZE))

/* Access to elements */

#define cdprq_top cdarr_first

/* Creation/Destruction */

/* Initializes the queue from NULL, pointer to
 * destructor and pointer to comparison function
 */
#define cdprq_init(prq, dtor_func, cmp_func) do { \
    cd__realloc_with_new_cap(prq, CDARR_INIT_CAP, CDPRQ_HEADER_SIZE); \
    cdarr_dtor(prq) = (cdarr_dtor_t)(dtor_func); \
    cdprq_cmp (prq) = (cdprq_cmp_t)(cmp_func); \
    cdarr_size(prq) = 0; \
} while (0)

/* Frees up the memory allocated for the queue */
#define cdprq_free(prq) do { \
    cdarr_clear(prq); \
    CDARR_FREE(cd__ptr_shift(prq, -CDPRQ_HEADER_SIZE)); \
    (prq) = 0; \
} while (0)

/* Items management */

/* Adds an item to the queue */
#define cdprq_push(prq, value) do { \
    size_t cd__cur, cd__pnt; \
    cdarr_push_back(prq, value); \
    cd__cur = cdarr_size(prq) - 1; \
    cd__pnt = (cd__cur - 1) / 2; \
    while (cd__cur > 0 && cdprq_cmp(prq)((prq) + cd__pnt, (prq) + cd__cur) < 0) { \
        cd__swap_by_ptr((prq) + cd__cur, (prq) + cd__pnt); \
        cd__cur = cd__pnt; cd__pnt = (cd__cur - 1) / 2; \
    } \
} while (0)

/* Deletes an item from the queue */
#define cdprq_pop(prq) do { \
    CDARR_ASSERT(cdarr_size(prq) > 0, "Deleting from an empty array"); \
    cd__swap_by_ptr(prq, (prq) + cdarr_size(prq) - 1); \
    cdarr_pop_back(prq); cd__heapify(prq, 0); \
} while (0)

#endif /* C_DYNAMIC_PRIORITY_QUEUE_H */