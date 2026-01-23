#ifndef JAVASCRIPT_OBJECT_NOTATION_H
#define JAVASCRIPT_OBJECT_NOTATION_H

#include <stddef.h>
#include <stdbool.h>

#define JC_TAB_SIZE     4
#define JC_INIT_STR_CAP 32
#define JC_INIT_ARR_CAP 16
#define JC_INIT_OBJ_CAP 16
#define JC_NUM_BUF_SIZE 256

typedef struct jvalue jvalue_t;

typedef enum jtype {
    JT_NULL = 0,
    JT_BOOLEAN,
    JT_INTEGER,
    JT_NUMBER,
    JT_STRING,
    JT_ARRAY,
    JT_OBJECT
} jtype_t;

typedef struct jarray {
    jvalue_t** values;
    size_t count;
} jarray_t;

typedef struct jentry {
    const char* key;
    jvalue_t* value;
} jentry_t;

typedef struct jobject {
    jentry_t* entries;
    size_t count;
} jobject_t;

struct jvalue {
    jtype_t type;
    union {
        bool        boolean;
        double      number;
        long long   integer;
        const char* string;
        jarray_t    array;
        jobject_t   object;
    } as;
};

typedef struct jreader {
    int (*next)(void*); // character on success, negative value on failure
    int (*peek)(void*); // character on success, negative value on failure
    void* ctx;
} jreader_t;

jvalue_t* jparse(jreader_t reader);
jvalue_t* jparse_cstr(const char* string);
jvalue_t* jparse_file(const char* filename);

jvalue_t* jat(jvalue_t* object, const char* key);

jvalue_t* jpath(jvalue_t* value, size_t depth, ...);
#define ji__Arg_count(_1, _2, _3, _4, _5, _6, _7, _8, _9, _a, _b, _c, _d, _e, _f, n, ...) n
#define ji__arg_count(...) ji__Arg_count(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, _)
#define jpath(value, ...) jpath((value), ji__arg_count(__VA_ARGS__), __VA_ARGS__)

void jprint(jvalue_t* value, unsigned level);
#define jprint(value) jprint((value), 0)

void jfree(jvalue_t* value);

#endif /* JAVASCRIPT_OBJECT_NOTATION_H */

#ifdef JSON_IMPLEMENTATION

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static bool jiisspace(int ch) {
    return ch ==  ' ' || ch == '\n'
        || ch == '\t' || ch == '\r';
}

static bool jiisdelim(int ch) {
    return ch  <  0  || jiisspace(ch)
        || ch == ',' || ch == ']' || ch == '}';
}

static bool jiisdec(int ch) {
    return '0' <= ch && ch <= '9';
}

static bool jiishex(int ch) {
    return ('0' <= ch && ch <= '9')
        || ('a' <= ch && ch <= 'f')
        || ('A' <= ch && ch <= 'F');
}

static unsigned jifromhex(int digit) {
    if ('0' <= digit && digit <= '9') return digit - '0';
    if ('a' <= digit && digit <= 'f') return digit - 'a' + 10;
    if ('A' <= digit && digit <= 'F') return digit - 'A' + 10;
    return -1;
}

static void jiskipws(jreader_t rdr) {
    while (true) {
        int ch = rdr.peek(rdr.ctx);
        if (!jiisspace(ch)) break;
        (void)rdr.next(rdr.ctx);
    }
}

typedef struct { char* ptr; size_t len, cap; } jisb_t;

static bool jisbrequire(jisb_t* sb, size_t require) {
    if (sb->len + require > sb->cap) {
        sb->cap += sb->cap ? sb->cap / 2 : JC_INIT_STR_CAP;
        void* new = realloc(sb->ptr, sb->cap);
        if (!new) return false;
        sb->ptr = new;
    }
    return true;
}

static bool jipushchar(int ch, jisb_t* sb) {
    if (!jisbrequire(sb, 1)) return false;
    sb->ptr[sb->len++] = (unsigned char)ch;
    return true;
}

static bool jipushutf8(unsigned cp, jisb_t* sb) {
    int count_bytes = 0;
    /**/ if (cp < 0x080) count_bytes = 1;
    else if (cp < 0x800) count_bytes = 2;
    else                 count_bytes = 3;

    if (!jisbrequire(sb, count_bytes)) return false;

    switch (count_bytes) {
        case 1:
            sb->ptr[sb->len++] = cp;
            break;
        case 2:
            sb->ptr[sb->len++] = 0xC0 | cp >> 6;
            sb->ptr[sb->len++] = 0x80 | (cp & 63);
            break;
        case 3:
            sb->ptr[sb->len++] = 0xE0 | cp >> 12;
            sb->ptr[sb->len++] = 0x80 | ((cp >> 6) & 63);
            sb->ptr[sb->len++] = 0x80 | (cp & 63);
            break;
    }

    return true;
}

static bool jiparsestring(jreader_t rdr, const char** strptr, bool inentry) {
    int ch; jisb_t sb = {0};
    if (rdr.next(rdr.ctx) != '"') goto error;
    ch = rdr.next(rdr.ctx);
    while (ch != '"') {
        if (ch < ' ') goto error;
        if (ch == '\\')
            switch (rdr.next(rdr.ctx)) {
                case '"' : if (!jipushchar( '"', &sb)) { goto error; } break;
                case '\\': if (!jipushchar('\\', &sb)) { goto error; } break;
                case '/' : if (!jipushchar( '/', &sb)) { goto error; } break;
                case 'b' : if (!jipushchar('\b', &sb)) { goto error; } break;
                case 'f' : if (!jipushchar('\f', &sb)) { goto error; } break;
                case 'n' : if (!jipushchar('\n', &sb)) { goto error; } break;
                case 'r' : if (!jipushchar('\r', &sb)) { goto error; } break;
                case 't' : if (!jipushchar('\t', &sb)) { goto error; } break;
                case 'u': {
                    unsigned hex = 0; int digit;
                    for (size_t i = 0; i < 4; i++)
                        if (jiishex(digit = rdr.next(rdr.ctx)))
                            hex = hex << 4 | jifromhex(digit);
                        else goto error;
                    if (!jipushutf8(hex, &sb)) goto error;
                } break;
                default: goto error;
            }
        else
            if (!jipushchar(ch, &sb)) goto error;
        ch = rdr.next(rdr.ctx);
    }

    ch = rdr.peek(rdr.ctx);
    if (!jiisdelim(ch) && (!inentry || ch != ':')) goto error;
    if (!jipushchar('\0', &sb)) goto error;

    void* cropped = realloc(sb.ptr, sb.len);
    if (!cropped) goto error;
    *strptr = cropped;

    return true;
error:
    free(sb.ptr);
    return false;
}

static bool jiparsekeyword(jreader_t rdr, const char* kw) {
    while (*kw) if (rdr.next(rdr.ctx) != *kw++) return false;
    return jiisdelim(rdr.peek(rdr.ctx));
}

static bool jipushvalue(jvalue_t* value, jarray_t* arr, size_t* cap) {
    if (arr->count >= *cap) {
        *cap += *cap ? *cap / 2 : JC_INIT_ARR_CAP;
        void* new = realloc(arr->values, *cap * sizeof *arr->values);
        if (!new) return false;
        arr->values = new;
    }

    arr->values[arr->count++] = value;
    return true;
}

static bool jipushentry(jentry_t entry, jobject_t* obj, size_t* cap) {
    if (obj->count >= *cap) {
        *cap += *cap ? *cap / 2 : JC_INIT_OBJ_CAP;
        void* new = realloc(obj->entries, *cap * sizeof *obj->entries);
        if (!new) return false;
        obj->entries = new;
    }

    obj->entries[obj->count++] = entry;
    return true;
}

static int jientrycmp(const void* lhs, const void* rhs) {
    const jentry_t *l = lhs, *r = rhs;
    return strcmp(l->key, r->key);
}

static jvalue_t* jiparsevalue(jreader_t rdr) {
    jvalue_t* value = malloc(sizeof *value);
    if (!value) return NULL;
    memset(value, 0, sizeof *value);

    jiskipws(rdr);
    switch (rdr.peek(rdr.ctx)) {
        case 'n': {
            value->type = JT_NULL;
            if (!jiparsekeyword(rdr, "null")) goto error;
        } break;
        case 'f': {
            value->type = JT_BOOLEAN;
            if (!jiparsekeyword(rdr, "false")) goto error;
            value->as.boolean = false;
        } break;
        case 't': {
            value->type = JT_BOOLEAN;
            if (!jiparsekeyword(rdr, "true")) goto error;
            value->as.boolean = true;
        } break;
        case '"': {
            value->type = JT_STRING;
            if (!jiparsestring(rdr, &value->as.string, false)) goto error;
        } break;
        case '[': {
            value->type = JT_ARRAY;
            (void)rdr.next(rdr.ctx);
            jiskipws(rdr);
            if (rdr.peek(rdr.ctx) == ']') {
                (void)rdr.next(rdr.ctx); break;
            }

            size_t cap = 0; int ch;
            do {
                jvalue_t* element = jiparsevalue(rdr);
                if (!element) goto error;
                if(!jipushvalue(element, &value->as.array, &cap)) goto error;
                ch = rdr.next(rdr.ctx);
                if (ch != ',' && ch != ']') goto error;
            } while (ch == ',');

            void* cropped = realloc(value->as.array.values,
                sizeof *value->as.array.values * value->as.array.count);
            if (!cropped) goto error;
            value->as.array.values = cropped;
        } break;
        case '{': {
            value->type = JT_OBJECT;
            (void)rdr.next(rdr.ctx);
            jiskipws(rdr);
            if (rdr.peek(rdr.ctx) == '}') {
                (void)rdr.next(rdr.ctx); break;
            }

            jentry_t entry;
            size_t cap = 0; int ch;
            do {
                memset(&entry, 0, sizeof entry);

                jiskipws(rdr);
                if (!jiparsestring(rdr, &entry.key, true)) goto error_obj;
                jiskipws(rdr);
                if (rdr.next(rdr.ctx) != ':') goto error_obj;
                entry.value = jiparsevalue(rdr);
                if (!entry.value) goto error_obj;
                if (!jipushentry(entry, &value->as.object, &cap)) goto error_obj;

                ch = rdr.next(rdr.ctx);
                if (ch != ',' && ch != '}') goto error;
            } while (ch == ',');

            void* cropped = realloc(value->as.object.entries,
                sizeof *value->as.object.entries * value->as.object.count);
            if (!cropped) goto error;
            value->as.object.entries = cropped;
            qsort(value->as.object.entries, value->as.object.count,
                sizeof *value->as.object.entries, jientrycmp);

            break;
        error_obj:
            free((void*)entry.key);
            jfree(entry.value);
            goto error;
        } break;
        default: {
            static char buffer[JC_NUM_BUF_SIZE];
            size_t size = 0; int ch;
            memset(buffer, 0, sizeof buffer);

            value->type = JT_INTEGER;
            if (rdr.peek(rdr.ctx) == '-') {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
            }

            ch = rdr.next(rdr.ctx);
            /*  */ if (ch == '0') {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = '0';
            } else if (jiisdec(ch)) {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = ch;
                while (jiisdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            } else goto error;

            if (rdr.peek(rdr.ctx) == '.') {
                value->type = JT_NUMBER;
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
                if (!jiisdec(rdr.peek(rdr.ctx))) goto error;
                while (jiisdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            }

            ch = rdr.peek(rdr.ctx);
            if (ch == 'e' || ch == 'E') {
                value->type = JT_NUMBER;
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
                ch = rdr.peek(rdr.ctx);
                if (ch == '+' || ch == '-') {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
                while (jiisdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            }

            if (!jiisdelim(rdr.peek(rdr.ctx))) goto error;
            char* end; errno = 0;
            if (value->type == JT_INTEGER)
                value->as.integer = strtoll(buffer, &end, 10);
            else
                value->as.number = strtod(buffer, &end);
            if (errno == ERANGE || buffer + size != end) goto error;
        } break;
    }
    jiskipws(rdr);

    return value;
error:
    jfree(value);
    return NULL;
}

jvalue_t* jparse(jreader_t rdr) {
    jvalue_t* json = jiparsevalue(rdr);
    if (!json) return json;
    if (rdr.next(rdr.ctx) < 0)
        return json;
    jfree(json);
    return NULL;
}

static int jirdrstrnext(void* ptr) {
    union { void* v; const char** s; } conv = {.v = ptr};
    return **conv.s ? *(*conv.s)++ : -1;
}

static int jirdrstrpeek(void* ptr) {
    union { void* v; const char** s; } conv = {.v = ptr};
    return **conv.s ? **conv.s : -1;
}

jvalue_t* jparse_cstr(const char* str) {
    if (!str) return NULL;
    return jparse((jreader_t){
        jirdrstrnext, jirdrstrpeek, &str
    });
}

static int jirdrfilenext(void* ptr) {
    union { void* v; FILE* f; } conv = {.v = ptr};
    return fgetc(conv.f);
}

static int jirdrfilepeek(void* ptr) {
    union { void* v; FILE* f; } conv = {.v = ptr};
    return ungetc(fgetc(conv.f), conv.f);
}

jvalue_t* jparse_file(const char* filename) {
    FILE* fd = fopen(filename, "r");
    if (!fd) return NULL;
    jvalue_t* json = jparse((jreader_t){
        jirdrfilenext, jirdrfilepeek, fd
    }); fclose(fd);
    return json;
}

jvalue_t* jat(jvalue_t* obj, const char* key) {
    if (!key || !obj || obj->type != JT_OBJECT) return NULL;
    jentry_t kentry = {0}; kentry.key = (char*)key;
    jentry_t* find = bsearch(&kentry,
        obj->as.object.entries, obj->as.object.count,
        sizeof *obj->as.object.entries, jientrycmp);
    return find ? find->value : NULL;
}

jvalue_t* (jpath)(jvalue_t* value, size_t depth, ...) {
    va_list args;
    va_start(args, depth);

    while (value && depth --> 0) {
        /*  */ if (value->type == JT_OBJECT) {
            const char* key = va_arg(args, const char*);
            value = jat(value, key);
        } else if (value->type == JT_ARRAY) {
            size_t index = va_arg(args, size_t);
            if (index >= value->as.array.count) goto error;
            value = value->as.array.values[index];
        } else
            goto error;
    }

    va_end(args);
    return value;
error:
    va_end(args);
    return NULL;
}

void (jprint)(jvalue_t* value, unsigned level) {
    if (!value) return;
    switch (value->type) {
        case JT_NULL: fputs("null", stdout); break;
        case JT_BOOLEAN: fputs(value->as.boolean ? "true" : "false", stdout); break;
        case JT_INTEGER: printf("%lli", value->as.integer); break;
        case JT_NUMBER: printf("%lg", value->as.number); break;
        case JT_STRING: printf("\"%s\"", value->as.string); break;
        case JT_ARRAY: {
            putchar('[');
            if (value->as.array.count == 0) { putchar(']'); break; }
            putchar('\n');
            for (size_t i = 0; i < value->as.array.count; i++) {
                printf("%*s", (level + 1) * JC_TAB_SIZE, "");
                (jprint)(value->as.array.values[i], level + 1);
                if (i < value->as.array.count - 1) putchar(',');
                putchar('\n');
            }
            printf("%*s]", level * JC_TAB_SIZE, "");
        } break;
        case JT_OBJECT: {
            putchar('{');
            if (value->as.object.count == 0) { putchar('}'); break; }
            putchar('\n');
            for (size_t i = 0; i < value->as.object.count; i++) {
                jentry_t entry = value->as.object.entries[i];
                printf("%*s\"%s\": ", (level + 1) * JC_TAB_SIZE, "", entry.key);
                (jprint)(entry.value, level + 1);
                if (i < value->as.object.count - 1) putchar(',');
                putchar('\n');
            }
            printf("%*s}", level * JC_TAB_SIZE, "");
        } break;
    }
}

void jfree(jvalue_t* value) {
    if (!value) return;
    switch (value->type) {
        case JT_NULL: case JT_BOOLEAN:
        case JT_INTEGER: case JT_NUMBER: break;

        case JT_STRING: free((void*)value->as.string); break;
        case JT_ARRAY: {
            for (size_t i = 0; i < value->as.array.count; i++)
                jfree(value->as.array.values[i]);
            free(value->as.array.values);
        } break;
        case JT_OBJECT: {
            for (size_t i = 0; i < value->as.object.count; i++) {
                free((void*)value->as.object.entries[i].key);
                jfree(value->as.object.entries[i].value);
            }
            free(value->as.object.entries);
        } break;
    }
    free(value);
}

#endif /* JSON_IMPLEMENTATION */