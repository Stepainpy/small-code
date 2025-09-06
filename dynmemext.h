#ifndef DYNAMIC_MEMORY_EXTENSIONS_H
#define DYNAMIC_MEMORY_EXTENSIONS_H

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <stddef.h>

#ifdef __GNUC__
#  include <stdlib.h>
#  define __gnuc_attr(x) __attribute__(x)
#else
#  define __gnuc_attr(x)
#endif

char* strdup (const char* str)              __gnuc_attr((malloc(free, 1)));
char* strndup(const char* str, size_t size) __gnuc_attr((malloc(free, 1)));

int  asprintf(char** restrict strptr, const char* restrict format, ...         ) __gnuc_attr((format(printf, 2, 3)));
int vasprintf(char** restrict strptr, const char* restrict format, va_list list) __gnuc_attr((format(printf, 2, 0)));

int  aswprintf(wchar_t** restrict strptr, const wchar_t* restrict format, ...);
int vaswprintf(wchar_t** restrict strptr, const wchar_t* restrict format, va_list list);

ssize_t getline(  char  ** lineptr, size_t* n, FILE* stream);
ssize_t getwline(wchar_t** lineptr, size_t* n, FILE* stream);

ssize_t getdelim(char** restrict lineptr, size_t* restrict n, int delimiter, FILE* stream);
ssize_t getwdelim(wchar_t** restrict lineptr, size_t* restrict n, wint_t delimiter, FILE* stream);

#endif /* DYNAMIC_MEMORY_EXTENSIONS_H */

#ifdef DYNMEMEXT_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>

char* strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* new = malloc(len + 1);
    if (!new) return NULL;
    return strcpy(new, str);
}

char* strndup(const char* str, size_t size) {
    if (!str) return NULL;
    size_t len = strlen(str);
    len = len < size ? len : size;
    char* new = calloc(1, len + 1);
    if (!new) return NULL;
    return memcpy(new, str, len);
}

int asprintf(char** restrict sp, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vasprintf(sp, fmt, args);
    va_end(args);
    return len;
}

int vasprintf(char** restrict sp, const char* restrict fmt, va_list args) {
    va_list acpy;
    va_copy(acpy, args);
    int len = vsnprintf(NULL, 0, fmt, acpy);
    va_end(acpy);
    if (len < 0) return -1;

    *sp = malloc(len + 1);
    if (!*sp) return -1;

    va_copy(acpy, args);
    len = vsprintf(*sp, fmt, acpy);
    va_end(acpy);
    return len;
}

int aswprintf(wchar_t** restrict sp, const wchar_t* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = vaswprintf(sp, fmt, args);
    va_end(args);
    return len;
}

int vaswprintf(wchar_t** restrict sp, const wchar_t* restrict fmt, va_list args) {
    va_list acpy;
    va_copy(acpy, args);
    int len = vsnwprintf(NULL, 0, fmt, acpy);
    va_end(acpy);
    if (len < 0) return -1;

    *sp = malloc((len + 1) * sizeof **sp);
    if (!*sp) return -1;

    va_copy(acpy, args);
    len = vswprintf(*sp, len + 1, fmt, acpy);
    va_end(acpy);
    return len;
}

ssize_t getline(char** lineptr, size_t* n, FILE* stream) {
    return getdelim(lineptr, n, '\n', stream);
}

ssize_t getwline(wchar_t** lineptr, size_t* n, FILE* stream) {
    return getwdelim(lineptr, n, L'\n', stream);
}

ssize_t getdelim(char** restrict lp, size_t* restrict n, int delim, FILE* stm) {
    size_t cap = 256, len = 0;
    *lp = realloc(NULL, cap);
    if (!*lp) return -1;

    for (int ch; (ch = fgetc(stm)) != EOF;) {
        if (len >= cap) {
            void* newp = realloc(*lp, cap *= 2);
            if (!newp) goto error;
            *lp = newp;
        }

        (*lp)[len++] = (unsigned char)ch;
        if (ch == delim) break;
    }

    if (len == 0) goto error;

    void* newp = realloc(*lp, len + 1);
    if (!newp) goto error;
    *lp = newp;
    (*lp)[len] = '\0';
    return *n = len;

error:
    free(*lp);
    return -1;
}

ssize_t getwdelim(wchar_t** restrict lp, size_t* restrict n, wint_t delim, FILE* stm) {
    size_t cap = 256, len = 0;
    *lp = realloc(NULL, cap * sizeof **lp);
    if (!*lp) return -1;

    for (wint_t ch; (ch = fgetwc(stm)) != WEOF;) {
        if (len >= cap) {
            void* newp = realloc(*lp, (cap *= 2) * sizeof **lp);
            if (!newp) goto error;
            *lp = newp;
        }

        (*lp)[len++] = (wchar_t)ch;
        if (ch == delim) break;
    }

    if (len == 0) goto error;

    void* newp = realloc(*lp, (len + 1) * sizeof **lp);
    if (!newp) goto error;
    *lp = newp;
    (*lp)[len] = L'\0';
    return *n = len;

error:
    free(*lp);
    return -1;
}

#endif /* DYNMEMEXT_IMPLEMENTATION */