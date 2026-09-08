#ifndef JAVASCRIPT_OBJECT_NOTATION_H
#define JAVASCRIPT_OBJECT_NOTATION_H

#include <stddef.h>
#include <stdbool.h>

#ifndef JSON_TAB_SIZE
#define JSON_TAB_SIZE 4
#endif

#define JSON_FMT_NUM "%lg"
#define JSON_FMT_INT "%lli"

typedef enum json_type_t {
    JSON_TYPE_NUL,
    JSON_TYPE_BLN,
    JSON_TYPE_INT,
    JSON_TYPE_NUM,
    JSON_TYPE_STR,
    JSON_TYPE_ARR,
    JSON_TYPE_OBJ,
} json_type_t;

typedef double    json_number_t;
typedef long long json_integer_t;

typedef struct json_value_t json_value_t;

typedef struct json_array_t {
    json_value_t** values;
    size_t count;
} json_array_t;

typedef struct json_entry_t {
    const char* key;
    json_value_t* value;
} json_entry_t;

typedef struct json_object_t {
    json_entry_t* entries;
    size_t count;
} json_object_t;

struct json_value_t {
    json_type_t type;
    union {
        bool           boolean;
        json_number_t  number;
        json_integer_t integer;
        const char*    string;
        json_array_t   array;
        json_object_t  object;
    } as;
};

typedef struct json_reader_t {
    int (*next)(void* ctx); // character on success, negative value on failure
    int (*peek)(void* ctx); // character on success, negative value on failure
    void* ctx;
} json_reader_t;

json_value_t* json_parse(json_reader_t reader);
json_value_t* json_parse_cstr(const char* string);
json_value_t* json_parse_file(const char* filename);

json_value_t* json_at(const json_value_t* object, const char* key);

json_value_t* json_path(const json_value_t* value, size_t depth, ...);
#define __jsoni_Arg_count(_1, _2, _3, _4, _5, _6, _7, _8, _9, _a, _b, _c, _d, _e, _f, n, ...) n
#define __jsoni_arg_count(...) __jsoni_Arg_count(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, _)
#define json_path(value, ...) json_path((value), __jsoni_arg_count(__VA_ARGS__), __VA_ARGS__)

void json_print(const json_value_t* value, unsigned level);
#define json_print(value) json_print((value), 0)

void json_free(json_value_t* value);

#endif /* JAVASCRIPT_OBJECT_NOTATION_H */

#ifdef JSON_IMPLEMENTATION

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define JSON_INIT_STR_CAP 32
#define JSON_INIT_ARR_CAP 16
#define JSON_INIT_OBJ_CAP 16
#define JSON_NUM_BUF_SIZE 256

static bool jsoni_isspace(int ch) {
    return ch ==  ' ' || ch == '\n'
        || ch == '\t' || ch == '\r';
}

static bool jsoni_isdelim(int ch) {
    return ch  <  0  || jsoni_isspace(ch)
        || ch == ',' || ch == ']' || ch == '}';
}

static bool jsoni_isdec(int ch) {
    return '0' <= ch && ch <= '9';
}

static bool jsoni_ishex(int ch) {
    return ('0' <= ch && ch <= '9')
        || ('a' <= ch && ch <= 'f')
        || ('A' <= ch && ch <= 'F');
}

static unsigned jsoni_fromhex(int digit) {
    if ('0' <= digit && digit <= '9') return digit - '0';
    if ('a' <= digit && digit <= 'f') return digit - 'a' + 10;
    if ('A' <= digit && digit <= 'F') return digit - 'A' + 10;
    return 0;
}

static void jsoni_skipws(json_reader_t rdr) {
    while (true) {
        int ch = rdr.peek(rdr.ctx);
        if (!jsoni_isspace(ch)) break;
        (void)rdr.next(rdr.ctx);
    }
}

typedef struct { char* ptr; size_t len, cap; } jsoni_sb_t;

static bool jsoni_sb_require(jsoni_sb_t* sb, size_t require) {
    if (sb->len + require > sb->cap) {
        sb->cap += sb->cap ? sb->cap / 2 : JSON_INIT_STR_CAP;
        void* newptr = realloc(sb->ptr, sb->cap);
        if (!newptr) return false;
        sb->ptr = newptr;
    }
    return true;
}

static bool jsoni_push_char(int ch, jsoni_sb_t* sb) {
    if (!jsoni_sb_require(sb, 1)) return false;
    sb->ptr[sb->len++] = (unsigned char)ch;
    return true;
}

static bool jsoni_push_utf8(unsigned cp, jsoni_sb_t* sb) {
    int bytes;
    /**/ if (cp < 0x080) bytes = 1;
    else if (cp < 0x800) bytes = 2;
    else                 bytes = 3;

    if (!jsoni_sb_require(sb, bytes)) return false;

    switch (bytes) {
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

static bool jsoni_parse_string(json_reader_t rdr, const char** strptr, bool inentry) {
    int ch; jsoni_sb_t sb = {0};
    if (rdr.next(rdr.ctx) != '"') goto error;
    ch = rdr.next(rdr.ctx);
    while (ch != '"') {
        if (ch < ' ') goto error;
        if (ch == '\\')
            switch (rdr.next(rdr.ctx)) {
                case '\"': if (!jsoni_push_char('\"', &sb)) { goto error; } break;
                case '\\': if (!jsoni_push_char('\\', &sb)) { goto error; } break;
                case  '/': if (!jsoni_push_char( '/', &sb)) { goto error; } break;
                case  'b': if (!jsoni_push_char('\b', &sb)) { goto error; } break;
                case  'f': if (!jsoni_push_char('\f', &sb)) { goto error; } break;
                case  'n': if (!jsoni_push_char('\n', &sb)) { goto error; } break;
                case  'r': if (!jsoni_push_char('\r', &sb)) { goto error; } break;
                case  't': if (!jsoni_push_char('\t', &sb)) { goto error; } break;
                case  'u': {
                    unsigned hex = 0; int digit;
                    for (size_t i = 0; i < 4; i++)
                        if (jsoni_ishex(digit = rdr.next(rdr.ctx)))
                            hex = hex << 4 | jsoni_fromhex(digit);
                        else goto error;
                    if (!jsoni_push_utf8(hex, &sb)) goto error;
                } break;
                default: goto error;
            }
        else
            if (!jsoni_push_char(ch, &sb)) goto error;
        ch = rdr.next(rdr.ctx);
    }

    ch = rdr.peek(rdr.ctx);
    if (!jsoni_isdelim(ch) && (!inentry || ch != ':')) goto error;
    if (!jsoni_push_char('\0', &sb)) goto error;

    void* cropped = realloc(sb.ptr, sb.len);
    if (!cropped) goto error;
    *strptr = cropped;

    return true;
error:
    free(sb.ptr);
    return false;
}

static bool jsoni_parse_keyword(json_reader_t rdr, const char* kw) {
    while (*kw) if (rdr.next(rdr.ctx) != *kw++) return false;
    return jsoni_isdelim(rdr.peek(rdr.ctx));
}

static bool jsoni_push_value(json_value_t* value, json_array_t* arr, size_t* cap) {
    if (arr->count >= *cap) {
        *cap += *cap ? *cap / 2 : JSON_INIT_ARR_CAP;
        void* newptr = realloc(arr->values, *cap * sizeof *arr->values);
        if (!newptr) return false;
        arr->values = newptr;
    }
    arr->values[arr->count++] = value;
    return true;
}

static bool jsoni_push_entry(json_entry_t entry, json_object_t* obj, size_t* cap) {
    if (obj->count >= *cap) {
        *cap += *cap ? *cap / 2 : JSON_INIT_OBJ_CAP;
        void* newptr = realloc(obj->entries, *cap * sizeof *obj->entries);
        if (!newptr) return false;
        obj->entries = newptr;
    }
    obj->entries[obj->count++] = entry;
    return true;
}

static int jsoni_entry_cmp(const void* lhs, const void* rhs) {
    const json_entry_t *l = lhs, *r = rhs;
    return strcmp(l->key, r->key);
}

static json_value_t* jsoni_parse_value(json_reader_t rdr) {
    json_value_t* value = malloc(sizeof *value);
    if (!value) return NULL;
    memset(value, 0, sizeof *value);

    jsoni_skipws(rdr);
    switch (rdr.peek(rdr.ctx)) {
        case 'n':
            value->type = JSON_TYPE_NUL;
            if (!jsoni_parse_keyword(rdr, "null")) goto error;
            break;

        case 'f':
            value->type = JSON_TYPE_BLN;
            if (!jsoni_parse_keyword(rdr, "false")) goto error;
            value->as.boolean = false;
            break;
        case 't':
            value->type = JSON_TYPE_BLN;
            if (!jsoni_parse_keyword(rdr, "true")) goto error;
            value->as.boolean = true;
            break;

        case '"':
            value->type = JSON_TYPE_STR;
            if (!jsoni_parse_string(rdr, &value->as.string, false)) goto error;
            break;

        case '[': {
            value->type = JSON_TYPE_ARR;
            (void)rdr.next(rdr.ctx);
            jsoni_skipws(rdr);
            if (rdr.peek(rdr.ctx) == ']') {
                (void)rdr.next(rdr.ctx); break;
            }

            size_t cap = 0; int ch;
            do {
                json_value_t* element = jsoni_parse_value(rdr);
                if (!element) goto error;
                if(!jsoni_push_value(element, &value->as.array, &cap)) goto error;
                ch = rdr.next(rdr.ctx);
                if (ch != ',' && ch != ']') goto error;
            } while (ch == ',');

            void* cropped = realloc(value->as.array.values,
                sizeof *value->as.array.values * value->as.array.count);
            if (!cropped) goto error;
            value->as.array.values = cropped;
        } break;

        case '{': {
            value->type = JSON_TYPE_OBJ;
            (void)rdr.next(rdr.ctx);
            jsoni_skipws(rdr);
            if (rdr.peek(rdr.ctx) == '}') {
                (void)rdr.next(rdr.ctx); break;
            }

            json_entry_t entry;
            size_t cap = 0; int ch;
            do {
                memset(&entry, 0, sizeof entry);

                jsoni_skipws(rdr);
                if (!jsoni_parse_string(rdr, &entry.key, true)) goto error_obj;
                jsoni_skipws(rdr);
                if (rdr.next(rdr.ctx) != ':') goto error_obj;
                entry.value = jsoni_parse_value(rdr);
                if (!entry.value) goto error_obj;
                if (!jsoni_push_entry(entry, &value->as.object, &cap)) goto error_obj;

                ch = rdr.next(rdr.ctx);
                if (ch != ',' && ch != '}') goto error;
            } while (ch == ',');

            void* cropped = realloc(value->as.object.entries,
                sizeof *value->as.object.entries * value->as.object.count);
            if (!cropped) goto error;
            value->as.object.entries = cropped;
            qsort(value->as.object.entries, value->as.object.count,
                sizeof *value->as.object.entries, jsoni_entry_cmp);

            for (size_t i = 1; i < value->as.object.count; i++)
                if (jsoni_entry_cmp(
                    value->as.object.entries + i - 1,
                    value->as.object.entries + i
                ) == 0) goto error;

            break;
        error_obj:
            free((void*)entry.key);
            json_free(entry.value);
            goto error;
        } break;

        default: {
            static char buffer[JSON_NUM_BUF_SIZE];
            size_t size = 0; int ch;
            memset(buffer, 0, sizeof buffer);

            value->type = JSON_TYPE_INT;
            if (rdr.peek(rdr.ctx) == '-') {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
            }

            ch = rdr.next(rdr.ctx);
            /*  */ if (ch == '0') {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = '0';
            } else if (jsoni_isdec(ch)) {
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = ch;
                while (jsoni_isdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            } else goto error;

            if (rdr.peek(rdr.ctx) == '.') {
                value->type = JSON_TYPE_NUM;
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
                if (!jsoni_isdec(rdr.peek(rdr.ctx))) goto error;
                while (jsoni_isdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            }

            ch = rdr.peek(rdr.ctx);
            if (ch == 'e' || ch == 'E') {
                value->type = JSON_TYPE_NUM;
                if (size >= sizeof buffer - 1) goto error;
                buffer[size++] = rdr.next(rdr.ctx);
                ch = rdr.peek(rdr.ctx);
                if (ch == '+' || ch == '-') {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
                while (jsoni_isdec(rdr.peek(rdr.ctx))) {
                    if (size >= sizeof buffer - 1) goto error;
                    buffer[size++] = rdr.next(rdr.ctx);
                }
            }

            if (!jsoni_isdelim(rdr.peek(rdr.ctx))) goto error;
            char* end; errno = 0;
            if (value->type == JSON_TYPE_INT)
                value->as.integer = strtoll(buffer, &end, 10);
            else
                value->as.number = strtod(buffer, &end);
            if (errno == ERANGE || buffer + size != end) goto error;
        } break;
    }
    jsoni_skipws(rdr);

    return value;
error:
    json_free(value);
    return NULL;
}

json_value_t* json_parse(json_reader_t rdr) {
    json_value_t* json = jsoni_parse_value(rdr);
    if (!json) return json;
    if (rdr.next(rdr.ctx) < 0)
        return json;
    json_free(json);
    return NULL;
}

static int jsoni_rdr_cstr_next(void* ptr) {
    union { void* v; const char** s; } conv = {.v = ptr};
    return **conv.s ? *(*conv.s)++ : -1;
}

static int jsoni_rdr_cstr_peek(void* ptr) {
    union { void* v; const char** s; } conv = {.v = ptr};
    return **conv.s ? **conv.s : -1;
}

json_value_t* json_parse_cstr(const char* str) {
    if (!str) return NULL;
    return json_parse((json_reader_t){
        jsoni_rdr_cstr_next,
        jsoni_rdr_cstr_peek,
        &str
    });
}

static int jsoni_rdr_file_next(void* ptr) {
    union { void* v; FILE* f; } conv = {.v = ptr};
    return fgetc(conv.f);
}

static int jsoni_rdr_file_peek(void* ptr) {
    union { void* v; FILE* f; } conv = {.v = ptr};
    return ungetc(fgetc(conv.f), conv.f);
}

json_value_t* json_parse_file(const char* filename) {
    FILE* fd = fopen(filename, "r");
    if (!fd) return NULL;
    json_value_t* json = json_parse((json_reader_t){
        jsoni_rdr_file_next,
        jsoni_rdr_file_peek,
        fd
    });
    fclose(fd);
    return json;
}

json_value_t* json_at(const json_value_t* obj, const char* key) {
    if (!key || !obj || obj->type != JSON_TYPE_OBJ) return NULL;
    json_entry_t kentry = { .key = key };
    json_entry_t* find = bsearch(&kentry,
        obj->as.object.entries, obj->as.object.count,
        sizeof *obj->as.object.entries, jsoni_entry_cmp);
    return find ? find->value : NULL;
}

json_value_t* (json_path)(const json_value_t* value, size_t depth, ...) {
    va_list args;
    va_start(args, depth);

    while (value && depth --> 0) {
        /*  */ if (value->type == JSON_TYPE_OBJ) {
            const char* key = va_arg(args, const char*);
            value = json_at(value, key);
        } else if (value->type == JSON_TYPE_ARR) {
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

void (json_print)(const json_value_t* value, unsigned level) {
    if (!value) return;
    switch (value->type) {
        case JSON_TYPE_BLN: fputs(value->as.boolean ? "true" : "false", stdout); break;
        case JSON_TYPE_INT: printf(JSON_FMT_INT, value->as.integer); break;
        case JSON_TYPE_NUM: printf(JSON_FMT_NUM, value->as.number ); break;
        case JSON_TYPE_STR: printf(  "\"%s\""  , value->as.string ); break;
        case JSON_TYPE_NUL: fputs("null", stdout); break;

        case JSON_TYPE_ARR: {
            putchar('[');
            if (value->as.array.count == 0) { putchar(']'); break; }
            if (JSON_TAB_SIZE) putchar('\n');
            for (size_t i = 0; i < value->as.array.count; i++) {
                printf("%*s", (level + 1) * JSON_TAB_SIZE, "");
                (json_print)(value->as.array.values[i], level + 1);
                if (i < value->as.array.count - 1) putchar(',');
                if (JSON_TAB_SIZE) putchar('\n');
            }
            printf("%*s]", level * JSON_TAB_SIZE, "");
        } break;

        case JSON_TYPE_OBJ: {
            putchar('{');
            if (value->as.object.count == 0) { putchar('}'); break; }
            if (JSON_TAB_SIZE) putchar('\n');
            for (size_t i = 0; i < value->as.object.count; i++) {
                json_entry_t entry = value->as.object.entries[i];
                printf("%*s\"%s\":", (level + 1) * JSON_TAB_SIZE, "", entry.key);
                if (JSON_TAB_SIZE) putchar(' ');
                (json_print)(entry.value, level + 1);
                if (i < value->as.object.count - 1) putchar(',');
                if (JSON_TAB_SIZE) putchar('\n');
            }
            printf("%*s}", level * JSON_TAB_SIZE, "");
        } break;
    }
}

void json_free(json_value_t* value) {
    if (!value) return;
    switch (value->type) {
        case JSON_TYPE_NUL: case JSON_TYPE_BLN:
        case JSON_TYPE_INT: case JSON_TYPE_NUM:
            // no release is required
            break;

        case JSON_TYPE_STR: free((void*)value->as.string); break;

        case JSON_TYPE_ARR:
            for (size_t i = 0; i < value->as.array.count; i++)
                json_free(value->as.array.values[i]);
            free(value->as.array.values);
            break;

        case JSON_TYPE_OBJ:
            for (size_t i = 0; i < value->as.object.count; i++) {
                free((void*)value->as.object.entries[i].key);
                json_free(value->as.object.entries[i].value);
            }
            free(value->as.object.entries);
            break;
    }
    free(value);
}

#endif /* JSON_IMPLEMENTATION */