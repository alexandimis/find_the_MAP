#include "random.h"

Rng_t rng;

static uint32_t pcg32_next(void){
    uint64_t old_state = rng.state;
    rng.state = old_state * UINT64_C(6364136223846793005) + rng.increment;
    uint32_t xor_shifted = (uint32_t)(((old_state >> 18u) ^ old_state) >> 27u);
    uint32_t rotation = (uint32_t)(old_state >> 59u);
    return (xor_shifted >> rotation) | (xor_shifted << (-rotation & 31u));
}

void rng_init(const uint64_t seed){
    rng.state = 0;
    rng.increment = UINT64_C(1442695040888963407);

    pcg32_next();
    rng.state += seed;
    pcg32_next();
}

uint32_t rng_next(void){
    return pcg32_next();
}

uint32_t rng_bounded(uint32_t bound){
    if(!bound) return 0;

    uint32_t threshold = (uint32_t)(-bound) % bound;

    uint32_t value = rng_next();
    while(value < threshold){
        value = rng_next();
    }

    return value % bound;
}
