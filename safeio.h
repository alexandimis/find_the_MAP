#ifndef SAFEIO_H
#define SAFEIO_H

#include <stdio.h>

// Safe allocators
void *xmalloc_impl(size_t size, const char *file, int line);
void *xcalloc_impl(size_t count, size_t size, const char *file, int line);

// Macros
#define xmalloc(size) xmalloc_impl((size), __FILE__, __LINE__)
#define xcalloc(count, size) xcalloc_impl((count), (size), __FILE__, __LINE__)


#endif