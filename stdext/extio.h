#ifndef EXTIO_H
#define EXTIO_H

#include <stdio.h>

#define floop(charvar, stream) \
while ((charvar = fgets(stream)) != EOF)

int  fpeek(FILE* stream);
long fsize(FILE* stream);

#if __STDC_VERSION__ >= 199409L
#include <wchar.h>

#define floopw(charvar, stream) \
while ((charvar = fgetwc(stream)) != WEOF)

wint_t fpeekw(FILE* stream);
#endif /* C95 */

#endif /* EXTIO_H */

#ifdef EXTIO_IMPLEMENTATION

int fpeek(FILE* stream) {
    int c = fgetc(   stream);
    return ungetc(c, stream);
}

#if __STDC_VERSION__ >= 199409L
wint_t fpeekw(FILE* stream) {
    wint_t wc = fgetwc(stream);
    return ungetwc(wc, stream);
}
#endif /* C95 */

long fsize(FILE* stream) {
    fpos_t pos; long size = -1;
    if (fgetpos(stream, &pos)) return -1;
    if (fseek(stream, 0, SEEK_END)) goto cleanup;
    if ((size = ftell(stream)) < 0) goto cleanup;
cleanup:
    if (fsetpos(stream, &pos)) return -1;
    return size;
}

#endif /* EXTIO_IMPLEMENTATION */