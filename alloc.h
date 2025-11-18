#ifndef CUSTOM_ALLOC_H
#define CUSTOM_ALLOC_H

#include <stddef.h>

void* custom_malloc(size_t size);
void custom_free(void* ptr);
void* custom_calloc(size_t num, size_t size);

#endif // CUSTOM_ALLOC_H
