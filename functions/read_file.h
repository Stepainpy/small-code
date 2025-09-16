#ifndef READ_FILE_H
#define READ_FILE_H

char* read_file(const char* filename);

#endif /* READ_FILE_H */

#ifdef READ_FILE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* filename) {
    long size; char* data = NULL;

    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    if (fseek(file, 0, SEEK_END)) goto cleanup;
    if ((size = ftell(file)) < 0) goto cleanup;
    if (fseek(file, 0, SEEK_SET)) goto cleanup;

    if ((data = malloc(size + 1)) != NULL) {
        size = fread(data, 1, size, file);
        data[size] = '\0';
    }

cleanup:
    fclose(file);
    return data;
}

#endif /* READ_FILE_IMPLEMENTATION */