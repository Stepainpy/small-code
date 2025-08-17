#include "ecs.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ECS_INIT_CAP 64

typedef struct { size_t capacity, size; } ecs_hdr_t;

#define ecs__get_header(str) \
    ((ecs_hdr_t*)(void*)((str) - sizeof(ecs_hdr_t)))
#define ecs__from_header(hdr) \
    ((ecs_t)(void*)(hdr) + sizeof(ecs_hdr_t))
#define ecs__min(a, b) ((a) < (b) ? (a) : (b))

size_t ecs_size(ecs_t str) {
    return str ? ecs__get_header(str)->size : 0;
}

size_t ecs_capacity(ecs_t str) {
    return str ? ecs__get_header(str)->capacity : 0;
}

static size_t ecs__new_cap(size_t old, size_t expect) {
    if (old < 2) old = ECS_INIT_CAP;
    while (old < expect) old += old / 2;
    return old;
}

static ecs_t ecs__create_with_cap(size_t cap) {
    cap = ecs__new_cap(0, cap);
    void* mem = malloc(sizeof(ecs_hdr_t) + cap);
    if (!mem) return NULL;

    ecs_hdr_t* hdr = mem;
    hdr->capacity = cap;
    hdr->size = 0;

    ecs_t str = ecs__from_header(hdr);
    memset(str, 0, cap);
    return str;
}

ecs_t ecs_create(void) {
    return ecs__create_with_cap(ECS_INIT_CAP);
}

ecs_t ecs_create_cstr(const char* cstr) {
    return ecs_create_data(cstr, cstr ? strlen(cstr) : 0);
}

ecs_t ecs_create_char(char ch, size_t count) {
    ecs_t str = ecs__create_with_cap(count + 1);
    if (str) {
        ecs__get_header(str)->size = count;
        memset(str, ch, count);
    }
    return str;
}

ecs_t ecs_create_data(const void* data, size_t size) {
    if (!data) return size == 0 ? ecs_create() : NULL;
    ecs_t str = ecs__create_with_cap(size + 1);
    if (str) {
        ecs__get_header(str)->size = size;
        memcpy(str, data, size);
    }
    return str;
}

ecs_t ecs_format(const char* fmt, ...) {
    if (!fmt) return ecs_create();

    va_list orig, copy;
    va_start(orig, fmt);
    va_copy(copy, orig);

    int count = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);

    ecs_t s = ecs__create_with_cap(count + 1);
    if (!s) { va_end(orig); return NULL; }

    ecs_hdr_t* hdr = ecs__get_header(s);
    hdr->size = vsnprintf(s, count + 1, fmt, orig);
    va_end(orig);
    return s;
}

ecs_t ecs_read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    long size; ecs_t str = NULL;
    if (fseek(file, 0, SEEK_END)) goto cleanup;
    if ((size = ftell(file)) < 0) goto cleanup;
    if (fseek(file, 0, SEEK_SET)) goto cleanup;

    str = ecs__create_with_cap(size + 1);
    if (!str) goto cleanup;

    ecs_hdr_t* hdr = ecs__get_header(str);
    size = fread(str, 1, size, file);
    hdr->size = size;
    str[size] = '\0';

cleanup:
    fclose(file);
    return str;
}

void ecs_destroy(ecs_t str) {
    if (str) free(str - sizeof(ecs_hdr_t));
}

ecs_t ecs_shrink_to_fit(ecs_t str) {
    if (!str) return NULL;
    ecs_hdr_t* hdr = ecs__get_header(str);
    hdr->capacity = hdr->size + 1;
    hdr = realloc(hdr, sizeof *hdr + hdr->capacity);
    if (!hdr) return NULL;
    return ecs__from_header(hdr);
}

ecs_t ecs_reserve(ecs_t str, size_t expect) {
    if (!str) return NULL;

    ecs_hdr_t* hdr = ecs__get_header(str);
    size_t new_cap = ecs__new_cap(hdr->capacity, expect + 1);
    void* mem = realloc(hdr, sizeof *hdr + new_cap);
    if (!mem) return NULL;

    hdr = mem; hdr->capacity = new_cap;
    return ecs__from_header(hdr);
}

ecs_t ecs_resize(ecs_t str, size_t size, char ph) {
    if (!str) return ecs_create_char(ph, size);
    ecs_hdr_t* hdr = ecs__get_header(str);
    /**/ if (size < hdr->size)
        return ecs_erase_data(str, size, hdr->size - size);
    else if (size > hdr->size) {
        str = ecs_reserve(str, size);
        if (!str) return NULL;
        hdr = ecs__get_header(str);
        memset(str + hdr->size, ph, size - hdr->size);
        str[hdr->size = size] = '\0';
    }
    return str;
}

ecs_t ecs_append_char(ecs_t str, char ch) {
    return ecs_append_data(str, &ch, 1);
}

ecs_t ecs_append_cstr(ecs_t str, const char* cstr) {
    return ecs_append_data(str, cstr, cstr ? strlen(cstr) : 0);
}

ecs_t ecs_append_data(ecs_t str, const void* data, size_t size) {
    if (!data) return str;
    str = ecs_reserve(str, ecs_size(str) + size);
    if (!str) return NULL;
    ecs_hdr_t* hdr = ecs__get_header(str);
    memcpy(str + hdr->size, data, size);
    str[hdr->size += size] = '\0';
    return str;
}

ecs_t ecs_prepend_char(ecs_t str, char ch) {
    return ecs_insert_char(str, 0, ch);
}

ecs_t ecs_prepend_cstr(ecs_t str, const char* cstr) {
    return ecs_insert_cstr(str, 0, cstr);
}

ecs_t ecs_prepend_data(ecs_t str, const void* data, size_t size) {
    return ecs_insert_data(str, 0, data, size);
}

ecs_t ecs_insert_char(ecs_t str, size_t index, char ch) {
    return ecs_insert_data(str, index, &ch, 1);
}

ecs_t ecs_insert_cstr(ecs_t str, size_t index, const char* cstr) {
    return ecs_insert_data(str, index, cstr, cstr ? strlen(cstr) : 0);
}

ecs_t ecs_insert_data(ecs_t str, size_t index, const void* data, size_t size) {
    if (!data || index >= ecs_size(str)) return str;
    str = ecs_reserve(str, ecs_size(str) + size);
    if (!str) return NULL;
    ecs_hdr_t* hdr = ecs__get_header(str);
    memmove(str + index + size, str + index, hdr->size - index + 1);
    memcpy(str + index, data, size); hdr->size += size;
    return str;
}

ecs_t ecs_clear(ecs_t str) {
    if (!str) return NULL;
    return ecs_erase_data(str, 0, ecs_size(str));
}

ecs_t ecs_erase_char(ecs_t str, size_t index) {
    return ecs_erase_data(str, index, 1);
}

ecs_t ecs_erase_data(ecs_t str, size_t index, size_t count) {
    if (!str || index >= ecs_size(str)) return str;
    ecs_hdr_t* hdr = ecs__get_header(str);
    if (index + count > hdr->size)
        count = hdr->size - index;
    memmove(str + index, str + index + count, count);
    hdr->size -= count;
    return str;
}

ecs_t ecs_replace(ecs_t str, char old, char new) {
    return ecs_replace_first_n(str, -1, old, new);
}

ecs_t ecs_replace_first_n(ecs_t str, size_t count, char old, char new) {
    if (!str || old == new) return str;
    ecs_hdr_t* hdr = ecs__get_header(str);
    while (count --> 0) {
        char* pch = memchr(str, old, hdr->size);
        if (!pch) break;
        *pch = new;
    }
    return str;
}