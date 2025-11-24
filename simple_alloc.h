#ifndef SIMEPL_ALLOC_H
#define SIMEPL_ALLOC_H

#include <stddef.h>

void* simple_malloc(size_t size);
void simple_free(void* ptr);
void* simple_realloc(void* ptr, size_t size);
void* simple_calloc(size_t num, size_t size);

#endif // SIMEPL_ALLOC_H
