#ifndef POOL_ALLOC_H
#define POOL_ALLOC_H

#include <stddef.h>

typedef struct {
    unsigned char* chunks;
    size_t chunk_size;
    size_t count;
    void* head;
} pool_t;

pool_t pool_init(void* buffer, size_t length, size_t sizeof_type);
void* pool_alloc(pool_t* pool);
void pool_free(pool_t* pool, void* ptr);

#endif // POOL_ALLOC_H

#ifdef POOL_IMPLEMENTATION

#include <string.h>

pool_t pool_init(void* buffer, size_t length, size_t sizeof_type) {
    pool_t out = {0};
    size_t chsz = sizeof(void*) > sizeof_type ? sizeof(void*) : sizeof_type;
    out.count = length / chsz;

    if (out.count == 0) return out;
    out.chunks = out.head = buffer;
    out.chunk_size = chsz;

    for (size_t i = 0; i < out.count; i++) {
        unsigned char* chunk = out.chunks + i * out.chunk_size;
        *(void**)chunk = (i == out.count - 1)
            ? NULL : chunk + out.chunk_size;
    }

    return out;
}

void* pool_alloc(pool_t* pool) {
    if (!pool || !pool->head) return NULL;
    void* out = pool->head;
    pool->head = *(void**)pool->head;
    return memset(out, 0, pool->chunk_size);
}

void pool_free(pool_t* pool, void* ptr) {
    if (!pool) return;
    if ((unsigned char*)ptr <  pool->chunks ||
        (unsigned char*)ptr >= pool->chunks +
        pool->count * pool->chunk_size) return;

    *(void**)ptr = pool->head; pool->head = ptr;
}

#endif // POOL_IMPLEMENTATION