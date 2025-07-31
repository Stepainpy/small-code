#ifndef BLOB_H
#define BLOB_H

typedef struct {
    void* data;
    long  size;
} blob_t;

blob_t blob_read_file(const char* filename);

#endif /* BLOB_H */

#ifdef BLOB_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

blob_t blob_read_file(const char* filename) {
    long size; void* data;
    blob_t out = {0};

    FILE* file = fopen(filename, "rb");
    if (!file) return out;

    if (fseek(file, 0, SEEK_END)) goto cleanup;
    if ((size = ftell(file)) < 0) goto cleanup;
    if (fseek(file, 0, SEEK_SET)) goto cleanup;

    if ((data = malloc(size + 1)) != NULL) {
        size = fread(data, 1, size, file);
        out.data = data; out.size = size;
        ((char*)out.data)[size] = '\0';
    }

cleanup:
    fclose(file);
    return out;
}

#endif /* BLOB_IMPLEMENTATION */