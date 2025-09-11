#ifndef READ_BLOB_H
#define READ_BLOB_H

#include <stddef.h>

typedef struct blob_t {
    void * data;
    size_t size;
} blob_t;

blob_t read_file_as_blob(const char* filename);

#endif /* READ_BLOB_H */

#ifdef READ_BLOB_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

blob_t read_file_as_blob(const char* filename) {
    long size; blob_t blob = {0};

    FILE* file = fopen(filename, "rb");
    if (!file) return blob;

    if (fseek(file, 0, SEEK_END)) goto cleanup;
    if ((size = ftell(file)) < 0) goto cleanup;
    if (fseek(file, 0, SEEK_SET)) goto cleanup;

    if ((blob.data = malloc(size)) != NULL)
        blob.size = fread(blob.data, 1, size, file);

cleanup:
    fclose(file);
    return blob;
}

#endif /* READ_BLOB_IMPLEMENTATION */