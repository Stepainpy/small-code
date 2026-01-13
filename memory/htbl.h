#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <string.h>

#if __STDC_VERSION__ >= 202311L
#  define htbli_typeof typeof
#elif defined(__GNUC__) || defined(_MSC_VER)
#  define htbli_typeof __typeof__
#endif

typedef uint64_t htbl_hash_t;
typedef htbl_hash_t (*htbl_hfn_t)(const char*);

typedef struct htbl_t htbl_t;
typedef struct htbl_value_t {
    void*  data;
    size_t size;
} htbl_value_t;

htbl_t*      htbl_init(htbl_hfn_t hash);
void*        htbl_push(htbl_t* tbl, const char* key, const void* data, size_t size);
htbl_value_t htbl_take(htbl_t* tbl, const char* key);
void         htbl_clip(htbl_t* tbl, const char* key);
void         htbl_free(htbl_t* tbl);

#define htbl_push_rval(tbl, key, value) \
    htbl_push((tbl), (key), &(htbli_typeof(value)){(value)}, sizeof(value))

#define htbl_push_cstr(tbl, key, str) \
    htbl_push((tbl), (key), (str), strlen(str) + 1)

#endif /* HASH_TABLE_H */

#ifdef HTBL_IMPLEMENTATION

#ifndef HTBL_INIT_CAP
#define HTBL_INIT_CAP 64
#endif

#include <stdlib.h>

typedef struct htbl_entry_t {
    struct htbl_entry_t* next;
    htbl_hash_t hash;
    size_t size;
    uint8_t data[];
} htbl_entry_t;

struct htbl_t {
    htbl_entry_t** entries;
    size_t count, capacity;
    htbl_hfn_t hash;
};

static htbl_hash_t htbli_dflt_hash(const char* key) {
    uint64_t out = UINT64_C(0xcbf29ce484222325);
    for (uint8_t byte; (byte = *key++);)
        out = (out ^ byte) * UINT64_C(0x00000100000001b3);
    return out;
}

htbl_t* htbl_init(htbl_hfn_t hash) {
    htbl_t* tbl = malloc(sizeof *tbl);
    if (!tbl) return NULL;
    memset(tbl, 0, sizeof *tbl);

    size_t init_byte_cap = sizeof *tbl->entries * HTBL_INIT_CAP;
    tbl->entries = malloc(init_byte_cap);
    if (!tbl->entries) { free(tbl); return NULL; }

    memset(tbl->entries, 0, init_byte_cap);
    tbl->hash = hash ? hash : htbli_dflt_hash;
    tbl->capacity = HTBL_INIT_CAP;

    return tbl;
}

void htbl_free(htbl_t* tbl) {
    if (tbl) {
        for (size_t i = 0; i < tbl->capacity; i++) {
            htbl_entry_t *next, *curr = tbl->entries[i];
            do {
                next = curr ? curr->next : NULL;
                free(curr);
            } while ((curr = next));
        }
        free(tbl->entries);
    }
    free(tbl);
}

static void htbli_extend(htbl_t* tbl) {
    size_t new_capacity = tbl->capacity * 2;
    htbl_entry_t** new_entries = malloc(sizeof *new_entries * new_capacity);
    if (!new_entries) return;

    memset(new_entries, 0, sizeof *new_entries * new_capacity);
    for (size_t i = 0; i < tbl->capacity; i++) {
        htbl_entry_t* curr = tbl->entries[i];
        for (; curr; curr = curr->next) {
            size_t j = curr->hash % new_capacity;
            curr->next = new_entries[j];
            new_entries[j] = curr;
        }
    }

    tbl->capacity = new_capacity;
    void* temp = tbl->entries;
    tbl->entries = new_entries;
    free(temp);
}

void* htbl_push(htbl_t* tbl, const char* key, const void* data, size_t size) {
    if (!tbl) return NULL;
    if (tbl->count > tbl->capacity)
        htbli_extend(tbl);

    htbl_entry_t* entry = malloc(sizeof *entry + size);
    if (!entry) return NULL;
    ++tbl->count;

    memcpy(entry->data, data, size);
    entry->hash = tbl->hash(key);
    entry->size = size;

    size_t i = entry->hash % tbl->capacity;
    entry->next = tbl->entries[i];
    tbl->entries[i] = entry;

    return entry->data;
}

htbl_value_t htbl_take(htbl_t* tbl, const char* key) {
    if (!tbl) return (htbl_value_t){0};

    htbl_hash_t hash = tbl->hash(key);
    size_t i = hash % tbl->capacity;
    htbl_entry_t* curr = tbl->entries[i];

    for (; curr; curr = curr->next)
        if (curr->hash == hash)
            return (htbl_value_t){
                .data = curr->data,
                .size = curr->size
            };

    return (htbl_value_t){0};
}

void htbl_clip(htbl_t* tbl, const char* key) {
    if (!tbl) return;

    htbl_hash_t hash = tbl->hash(key);
    size_t i = hash % tbl->capacity;
    htbl_entry_t** curr = tbl->entries + i;

    for (; *curr; curr = &(*curr)->next)
        if ((*curr)->hash == hash) {
            htbl_entry_t* erased = *curr;
            *curr = (*curr)->next;
            --tbl->count;
            free(erased);
            break;
        }
}

#endif /* HTBL_IMPLEMENTATION */