#ifndef NULL_ASSERT_ALLOC_H
#define NULL_ASSERT_ALLOC_H

#include <stddef.h>

#define naalloc(size) naalloc_fn((size), __FILE__, __LINE__)
void* naalloc_fn(size_t size, const char* file, int line);

#endif /* NULL_ASSERT_ALLOC_H */

#ifdef NAALLOC_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

void* naalloc_fn(size_t size, const char* file, int line) {
    void* memory = malloc(size);
    if (memory) return memory;
    fprintf(stderr, "%s:%i: No memory\n", file, line);
    exit(1);
}

#endif /* NAALLOC_IMPLEMENTATION */