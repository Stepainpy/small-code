#ifndef EXTENDABLE_C_STRING_H
#define EXTENDABLE_C_STRING_H

#include <stddef.h>

typedef char* ecs_t;

/* Access to properties */

size_t ecs_size(ecs_t str);
size_t ecs_capacity(ecs_t str);

/* Creation and destruction */

ecs_t ecs_create(void);
ecs_t ecs_create_cstr(const char* cstr);
ecs_t ecs_create_char(char ch, size_t count);
ecs_t ecs_create_data(const void* data, size_t size);

ecs_t ecs_format(const char* format, ...);
ecs_t ecs_read_file(const char* filename);

void ecs_destroy(ecs_t str);

/* Memory management */

ecs_t ecs_shrink_to_fit(ecs_t str);
ecs_t ecs_reserve(ecs_t str, size_t expect);
ecs_t ecs_resize(ecs_t str, size_t size, char placeholder);

/* Positive modifications */

ecs_t ecs_insert_char(ecs_t str, size_t index, char ch);
ecs_t ecs_insert_cstr(ecs_t str, size_t index, const char* cstr);
ecs_t ecs_insert_data(ecs_t str, size_t index, const void* data, size_t size);

ecs_t ecs_append_char(ecs_t str, char ch);
ecs_t ecs_append_cstr(ecs_t str, const char* cstr);
ecs_t ecs_append_data(ecs_t str, const void* data, size_t size);

ecs_t ecs_prepend_char(ecs_t str, char ch);
ecs_t ecs_prepend_cstr(ecs_t str, const char* cstr);
ecs_t ecs_prepend_data(ecs_t str, const void* data, size_t size);

/* Negative modifications */

ecs_t ecs_clear(ecs_t str);
ecs_t ecs_erase_char(ecs_t str, size_t index);
ecs_t ecs_erase_data(ecs_t str, size_t index, size_t count);

/* Positive/Neutral/Negative modifications */

ecs_t ecs_replace(ecs_t str, const char* old, const char* new);

#endif /* EXTENDABLE_C_STRING_H */