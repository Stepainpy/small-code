#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <stddef.h>

typedef struct pool pool_t;

pool_t* pool_init(size_t count, size_t sizeof_obj);
void* pool_alloc(pool_t* pool);
void pool_free(pool_t* pool, void* ptr);
void pool_free_all(pool_t* pool);

#endif /* POOL_ALLOCATOR_H */

#ifdef POOL_IMPLEMENTATION

#include <stdlib.h>

struct pool {
    struct pool_chunk {
        struct pool_chunk* next;
    }* head;
};

pool_t* pool_init(size_t count, size_t objsz) {
    pool_t* pool = NULL; char* mem; size_t memsz;
    if (!count || !objsz) return pool;

    objsz = objsz > sizeof(struct pool_chunk)
          ? objsz : sizeof(struct pool_chunk);
    memsz = sizeof *pool * 2 + objsz * count;

    if ((mem = malloc(memsz)) != NULL) {
        void *curr = mem + memsz, *prev = NULL;
        while (count --> 0) {
            curr = (char*)curr - objsz;
            ((struct pool_chunk*)curr)->next = prev;
            prev = curr;
        }
        pool = (pool_t*)(void*)mem;
        pool->head = prev;
    } return pool;
}

void pool_free_all(pool_t* pool) { free(pool); }

void pool_free(pool_t* pool, void* ptr) {
    if (!pool || !ptr) return;
    ((struct pool_chunk*)ptr)->next = pool->head;
    pool->head = ptr;
}

void* pool_alloc(pool_t* pool) {
    void* mem = NULL;
    if (!pool || !pool->head) return mem;
    mem = pool->head;
    pool->head = pool->head->next;
    return mem;
}

#endif /* POOL_IMPLEMENTATION */