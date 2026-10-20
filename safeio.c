#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "safeio.h"

void *xmalloc_impl(size_t size, const char *file, int line){
    void *ptr = malloc(size);

    if(!ptr){
        fprintf(stderr, "%s:%d: xmalloc error: failed to allocate %zu bytes", file, line, size);

        if(errno != 0){
            fprintf(stderr, ": %s\n", strerror(errno));
        }
        exit(EXIT_FAILURE);
    }

    return ptr;
}

void *xcalloc_impl(size_t count, size_t size, const char *file, int line){
    void *ptr = calloc(count, size);

    if(!ptr){
        fprintf(stderr, "%s:%d: xmalloc error: failed to allocate %zu bytes", file, line, count*size);

        if(errno != 0){
            fprintf(stderr, ": %s\n", strerror(errno));
        }
        exit(EXIT_FAILURE);
    }

    return ptr;
}