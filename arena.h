#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H

#include <stddef.h>

typedef struct arena arena_t;

arena_t* arena_init(size_t capacity);
void* arena_alloc(arena_t* arena, size_t size);
void* arena_alloc_align(arena_t* arena, size_t size, size_t align);
void arena_free(arena_t* arena);

#endif /* ARENA_ALLOCATOR_H */

#ifdef ARENA_IMPLEMENTATION

#include <stdlib.h>

struct arena { char *begin, *end; };

arena_t* arena_init(size_t cap) {
    arena_t* arena = NULL;
    char* mem = malloc(sizeof *arena + cap);
    if (!mem) return arena;
    arena = (arena_t*)(void*)mem;
    arena->begin = mem + sizeof *arena;
    arena->end   = mem + sizeof *arena + cap;
    return arena;
}

void arena_free(arena_t* arena) {
    free(arena);
}

void* arena_alloc(arena_t* arena, size_t size) {
    return arena_alloc_align(arena, size, 1);
}

void* arena_alloc_align(arena_t* arena, size_t size, size_t align) {
    void* mem = NULL;
    if (!arena || size == 0) return NULL;
    if (!(align && (align & (align - 1)) == 0)) return NULL;
    if ((uintptr_t)arena->begin % align != 0)
        arena->begin += align - (uintptr_t)arena->begin % align;
    if (arena->begin <= arena->end && (size_t)(arena->end - arena->begin) >= size) {
        mem = arena->begin;
        arena->begin += size;
    }
    return mem;
}

#endif /* ARENA_IMPLEMENTATION */