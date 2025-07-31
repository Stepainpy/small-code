#ifndef SINK_H
#define SINK_H

#include <stdio.h>

typedef struct sink_t sink_t;

sink_t* snk_init_file(FILE* file);
sink_t* snk_init_buffer(void* buffer, size_t capacity);

void snk_destroy(sink_t* snk); /* only free 'snk' */
void snk_close  (sink_t* snk); /* close file and free 'snk' */

/* 1 upon success, 0 otherwise */

int snk_put(sink_t* snk, unsigned char byte);
int snk_write(sink_t* snk, const void* buffer, size_t count);

#endif /* SINK_H */

#ifdef SINK_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

struct sink_t {
    size_t capacity;
    union {
        unsigned char* buffer;
        FILE* file;
    } where;
};

sink_t* snk_init_file(FILE* file) {
    sink_t* snk;
    if (!file) return NULL;
    snk = malloc(sizeof *snk);
    if (snk) {
        snk->capacity = SIZE_MAX;
        snk->where.file = file;
    }
    return snk;
}

sink_t* snk_init_buffer(void* buf, size_t cap) {
    sink_t* snk;
    if (!buf && cap > 0) return NULL;
    snk = malloc(sizeof *snk);
    if (snk) {
        snk->capacity = cap;
        snk->where.buffer = buf;
    }
    return snk;
}

void snk_destroy(sink_t* snk) {
    free(snk);
}

void snk_close(sink_t* snk) {
    if (snk && snk->capacity == SIZE_MAX)
        fclose(snk->where.file);
    snk_destroy(snk);
}

int snk_put(sink_t* snk, unsigned char byte) {
    if (!snk) return 0;
    if (snk->capacity == SIZE_MAX) {
        return fputc(byte, snk->where.file) != EOF;
    } else {
        if (snk->capacity > 0) {
            --snk->capacity;
            *snk->where.buffer++ = byte;
            return 1;
        } else
            return 0;
    }
}

int snk_write(sink_t* snk, const void* buf, size_t count) {
    if (!snk || (!buf && count > 0)) return 0;
    if (snk->capacity == SIZE_MAX) {
        return fwrite(buf, 1, count, snk->where.file) == count;
    } else {
        if (snk->capacity < count) return 0;
        memcpy(snk->where.buffer, buf, count);
        snk->capacity     -= count;
        snk->where.buffer += count;
        return 1;
    }
}

#endif /* SINK_IMPLEMENTATION */