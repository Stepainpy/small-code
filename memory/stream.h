#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>

typedef struct stream_t stream_t;

stream_t* stm_init_file(FILE* file);
stream_t* stm_init_string(const char* string);
stream_t* stm_init_buffer(const void* buffer, size_t length);

void stm_destroy(stream_t* stm); /* only free 'stm' */
void stm_close  (stream_t* stm); /* close file and free 'stm' */

/* get and peek semantic:
 * if stm is NULL then return EOF
 * if no more byte then return EOF
 */

int stm_get (stream_t* stm);
int stm_peek(stream_t* stm);

size_t stm_read(stream_t* stm, void* buffer, size_t count);

#endif /* STREAM_H */

#ifdef STREAM_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

struct stream_t {
    size_t count;
    union {
        const unsigned char* buffer;
        FILE* file;
    } source;
};

stream_t* stm_init_file(FILE* file) {
    stream_t* stm;
    if (!file) return NULL;
    stm = malloc(sizeof *stm);
    if (stm) {
        stm->count = SIZE_MAX;
        stm->source.file = file;
    }
    return stm;
}

stream_t* stm_init_string(const char* str) {
    stream_t* stm;
    if (!str) return NULL;
    stm = malloc(sizeof *stm);
    if (stm) {
        stm->count = strlen(str);
        stm->source.buffer =
            (const unsigned char*)str;
    }
    return stm;
}

stream_t* stm_init_buffer(const void* buf, size_t len) {
    stream_t* stm;
    if (!buf && len > 0) return NULL;
    stm = malloc(sizeof *stm);
    if (stm) {
        stm->count = len;
        stm->source.buffer = buf;
    }
    return stm;
}

void stm_destroy(stream_t* stm) {
    free(stm);
}

void stm_close(stream_t* stm) {
    if (stm && stm->count == SIZE_MAX)
        fclose(stm->source.file);
    stm_destroy(stm);
}

int stm_peek(stream_t* stm) { int c;
    if (!stm) return EOF;
    if (stm->count == SIZE_MAX) {
            c = fgetc(   stm->source.file);
        return ungetc(c, stm->source.file);
    } else {
        if (stm->count > 0)
            return *stm->source.buffer;
        else return EOF;
    }
}

int stm_get(stream_t* stm) {
    if (!stm) return EOF;
    if (stm->count == SIZE_MAX) {
        return fgetc(stm->source.file);
    } else {
        if (stm->count > 0) {
            --stm->count;
            return *stm->source.buffer++;
        } else
            return EOF;
    }
}

size_t stm_read(stream_t* stm, void* buffer, size_t count) {
    size_t min = stm->count < count ? stm->count : count;
    if (!stm || !buffer || count == 0) return 0;
    if (stm->count == SIZE_MAX) {
        return fread(buffer, 1, count, stm->source.file);
    } else {
        memcpy(buffer, stm->source.buffer, min);
        stm->count         -= min;
        stm->source.buffer += min;
        return min;
    }
}

#endif /* STREAM_IMPLEMENTATION */