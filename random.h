#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct{
    uint64_t state;
    uint64_t increment;
}Rng_t;


void rng_init(const uint64_t seed);
uint32_t rng_next(void);
uint32_t rng_bounded(uint32_t bound);


#endif