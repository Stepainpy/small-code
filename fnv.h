#ifndef FOWLER_NOLL_VO_HASH_H
#define FOWLER_NOLL_VO_HASH_H

#include <stddef.h>

#ifndef UINT32_MAX
#  include <limits.h>
#  define UINT32_MAX 0xFFFFFFFF
#    if  UINT_MAX == UINT32_MAX
typedef unsigned int  uint32_t;
#  elif ULONG_MAX == UINT32_MAX
typedef unsigned long uint32_t;
#  endif
#endif

#ifndef UINT64_MAX
#  include <limits.h>
#  define UINT64_MAX 0xFFFFFFFFFFFFFFFFULL
#    if      ULONG_MAX == UINT64_MAX
typedef unsigned long      uint64_t;
#  elif ULONG_LONG_MAX == UINT64_MAX
typedef unsigned long long uint64_t;
#  endif
#endif

uint32_t fnv1_32(const void* data, size_t size);
uint32_t fnv1a32(const void* data, size_t size);

uint64_t fnv1_64(const void* data, size_t size);
uint64_t fnv1a64(const void* data, size_t size);

#endif /* FOWLER_NOLL_VO_HASH_H */

#ifdef FNV_IMPLEMENTATION

uint32_t fnv1_32(const void* data, size_t size) {
    const unsigned char* byte = data;
    uint32_t hash = 0x811c9dc5;
    while (size --> 0) {
        hash *= 0x1000193;
        hash ^= *byte++;
    }
    return hash;
}

uint32_t fnv1a32(const void* data, size_t size) {
    const unsigned char* byte = data;
    uint32_t hash = 0x811c9dc5;
    while (size --> 0) {
        hash ^= *byte++;
        hash *= 0x1000193;
    }
    return hash;
}

uint64_t fnv1_64(const void* data, size_t size) {
    const unsigned char* byte = data;
    uint64_t hash = 0xcbf29ce484222325ull;
    while (size --> 0) {
        hash *= 0x100000001b3ull;
        hash ^= *byte++;
    }
    return hash;
}

uint64_t fnv1a64(const void* data, size_t size) {
    const unsigned char* byte = data;
    uint64_t hash = 0xcbf29ce484222325ull;
    while (size --> 0) {
        hash ^= *byte++;
        hash *= 0x100000001b3ull;
    }
    return hash;
}

#endif /* FNV_IMPLEMENTATION */